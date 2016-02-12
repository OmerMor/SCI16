/*
 * RESOURCE - Code to locate and load (if needed) resources for SCI
 *
 * Work started 7/30/87 by Robert E. Heitman for Sierra On-Line, Inc.
 *
 * All resource types (except MEMORY) are loaded from disk.
 * All references to a resource must be through the ResLoad function.
 * Memory resources are different in that the resId is the hunk size.
 *
 */

#include	"resource.h"

#include	"altres.h"
#include	"config.h"
#include	"debug.h"
#include	"event.h"
#include	"fileio.h"
#include	"flist.h"
#include	"memmgr.h"
#include	"mouse.h"
#include	"resname.h"
#include	"start.h"
#include	"dialog.h"
#include	"errmsg.h"
#include "graph.h"

#include	"io.h"
#include	"stdio.h"
#include "stdlib.h"
#include	"string.h"
#include	"volload.h"

FList	loadList;
bool	checkingLoadLinks = FALSE;
word	resourceCursorType = 0;

static Handle	patches = NULL;
static bool		getmemFailNonfatal = FALSE;

#if defined(DEBUG)

ubyte	theResType;
uint	theResId;

#define SetResInfo(type, id)	theResType = type; theResId = id;
#define ClearResInfo()			theResType = 0;    theResId = 0;

#else

#define SetResInfo(type, id)	
#define ClearResInfo()			

#endif

Handle
ResLoad(ubyte resType,uint resId)
{
	// return handle to resource

	LoadLink far** scan;
	bool				failflag;

	switch (resType) 
      {
		case RES_MEM:
		/* memory is never in the list */
			failflag = getmemFailNonfatal;
			getmemFailNonfatal = 0;
			scan = (LoadLink far * *) GetResHandle(sizeof(LoadLink));
			getmemFailNonfatal = failflag;
			/* if resType == RES_MEM, then resID is size of MEMORY needed */
			if (!((*scan)->lData.data = GetResHandle(resId))) {
			   DisposeHandle( (Handle) scan);
				return((Handle)0);
				}
			/* resId now becomes the resId for disposal purposes */
			resId = Pseudo((*scan)->lData.data);
         /* store size in LoadLink */
			(*scan)->size = HandleSize((*scan)->lData.data);
         /* memory types are always kept in standard memory */
			(*scan)->altResMem = 0;
			/* memory resources are locked by default */
			(*scan)->lock = LOCKED;
         CheckHunkUse(0);
			break;

		case RES_AUDIO:
		case RES_WAVE:
			return 0;

		default:
		/* all others must be in list or load from disk */
			if (scan = FindResEntry(resType, resId))
            {
				/* move to front of list */
				FMoveToFront(&loadList, Pseudo(scan));

            if ((*scan)->altResMem)
					/* Although loaded, this resource MAY be in alternate
					   memory.  If this is the case it must be moved to main RAM */
               {
         	   CheckHunkUse((*scan)->size / 16);
					return AltResMemRead(scan);
               }
				else
               /* Found resource in standard memory */
               {
		         if ((*scan)->lock == LOCKED)
         	      CheckHunkUse(0);
               else   
         	      CheckHunkUse((*scan)->size / 16);
			   	return((*scan)->lData.data);
               }
			   }

         /* need to load this resource from disk */
			/* get a link for this resource */
			scan = (LoadLink far * *) GetResHandle(sizeof(LoadLink));

			/* If tracking resources, write out what it is that we need. */
			if (trackResUse)
				WriteResUse(resType,resId);

		   SetResCursor(DiskCursor);
		   SetResInfo(resType, resId);
			(*scan)->lData.data = DoLoad(resType, resId);
			ClearResInfo();
		   SetResCursor(ArrowCursor);

			if (!(*scan)->lData.data)
            {
				DisposeHandle( (Handle) scan);
				return 0;
		      }

			/* all other resources are purgeable */
			(*scan)->lock = UNLOCKED;
			(*scan)->size = HandleSize((*scan)->lData.data);
			(*scan)->altResMem = 0;
	      CheckHunkUse((*scan)->size / 16);
			break;
		}

	/* link is prepared.  Add it to front and identify it */
	FAddToFront(&loadList, Pseudo(scan));
	
	(*scan)->type = resType;
	(*scan)->id = resId;

   /* checksum is used when the X option is chosen */
   if (checkingLoadLinks)
      {
	   (*scan)->checkSum = 0x44;
      }
	return (*scan)->lData.data;
}

void
ResUnLoad(
	ubyte	resType,
	uword resId)
{
	/* release this resource */

	LoadLink far **	scan;
	LoadLink far **	tmp;

   if (resId != ALL_IDS)
      {
	   if (scan = FindResEntry(resType, resId)) {
         if ((*scan)->altResMem)
         	AltResMemFree(scan);
		      /* free the data from standard memory */
         else if ((*scan)->lData.data)
	      	DisposeHandle((*scan)->lData.data);
		   /* delete the node and dispose of node memory */
		   FDeleteNode(&loadList, Pseudo(scan));
		   DisposeHandle( (Handle) scan);
      }
   } else {
	   for	(
			   scan = (LoadLink far * *) Native(FFirstNode(&loadList));
			   scan;
			   scan = tmp
			   )
		   {
		   tmp = (LoadLink far * *) Native(FNextNode(Pseudo(scan)));
		   if ((*scan)->type == resType) ResUnLoad(resType, (*scan)->id);
		   }
      }
}

void
ResLock(
	ubyte	resType,
	uword	resId,
	bool	yes)
{
	LoadLink far * *	scan;
	LoadLink far * *	tmp;

   if (resId != ALL_IDS)
      {
	   if (scan = FindResEntry(resType, resId))
         {
		   if (yes)	/* set the lock byte */
			   (*scan)->lock = LOCKED;
		   else
			   (*scan)->lock = UNLOCKED;
         }
      }
   else 
      {
	   for	(
			   scan = (LoadLink far * *) Native(FFirstNode(&loadList));
			   scan;
			   scan = tmp
			   )
		   {
		   tmp = (LoadLink far * *) Native(FNextNode(Pseudo(scan)));
		   if ((*scan)->type == resType) ResLock(resType, (*scan)->id, yes);
		   }
      }
   }

bool
PurgeLast()
{
	/* Copy last UNLOCKED node in list to alternate memory
      (don't purge resType:resId because thats what
      will be loaded into standard memory).
      Return TRUE if nothing can be purged. */

	LoadLink far **	scan;
	
	/*	find last resource that is not locked or in ARM	*/
	for (scan = (LoadLink far * *) Native(FLastNode(&loadList));
		  scan && ((*scan)->lock || (*scan)->altResMem);
		  scan = (LoadLink far * *) Native(FPrevNode(Pseudo(scan)))
		 )
		;
	if (!scan)
		return TRUE;
		
   /* if resource is RES_MEM, or no alternate memory available
   	we have to simply purge it */
   if ((*scan)->type == RES_MEM || !useAltResMem || !AltResMemWrite(scan)) {
      /* free the data, delete the node and dispose of node memory */
	   DisposeHandle((*scan)->lData.data);
	   FDeleteNode(&loadList, Pseudo(scan));
	   DisposeHandle( (Handle) scan);
	}
   return FALSE;
}

LoadLink far**
FindResEntry(
	ubyte	resType,
	uword	resId)
{
	LoadLink far * *scan;

/* search loadList for this type and id */
	for	(
			scan = (LoadLink far * *) Native(FFirstNode(&loadList));
			scan;
			scan = (LoadLink far * *) Native(FNextNode(Pseudo(scan)))
			)
	   {
		/* if type & id match we break */
		if ((*scan)->type == resType && (*scan)->id == resId)
			break;
	   }
	return(scan);
}

Handle
GetResHandle(
	uword	size)
{
	/* Return a Handle in concert with the Resource Manager
	** purge resource list if necessary
	*/

	Handle			hndl;

	while (!(hndl = RNewHandle(size)))
      {
		if (PurgeLast())
         {
			if (getmemFailNonfatal)
				return(hndl);
         if (GetHandle())
            {
#if defined(DEBUG)
				LoadLink far**	scan;

	         theGame = 0; /* Added to prevent Language error message */
            /* make some room so that resources can be shown */
		      scan = (LoadLink far * *) Native(FLastNode(&loadList));
		      while (!FIsFirstNode(Pseudo(scan)))
               {
				   scan = (LoadLink far * *) Native(FPrevNode(Pseudo(scan)));
			      if ((*scan)->altResMem)
						continue;
		         if ((*scan)->lData.data) DisposeHandle((*scan)->lData.data);
			      }
            RAlert(E_NO_HUNK_RES, ResNameMake(theResType, theResId));
	         Resources();
#endif
				Panic(E_NO_HUNK);
            }
         else
            {
				Panic(E_NO_HANDLES);
            }
		   }
	   }
	return(hndl);
}

Handle
LoadHandle(
	strptr	fileName)
{
	Handle	theHandle;
	long		len;
	int		fd;

	/* Open the file.
	 */
	if ((fd = open(fileName, 0)) == -1)
		return (NULL);

	/* Get a handle to storage for it.
	 */
	len = (long) lseek(fd, 0L, LSEEK_END);	/* find length to eof */
	lseek(fd, 0L, LSEEK_BEG);					/* rewind to beg of file */
	theHandle = NeedHandle( (uint) len );
	readfar(fd, theHandle, (uint) len);
	close(fd);

	return (theHandle);
}



void
SetResCursor(
	ResCursor	cursor)
{
#if !defined(DEBUG)
	cursor = cursor;
#else
	static char cursorSaved;

#define CURBUFSIZE	910
	/* This is where the current dursor data will get stored */
	static char PrevIcon[CURBUFSIZE] = 
   {
	11, 00,
	16, 00,
	01, 00,
	01, 00,
	10, 00,
	0x00,0x00,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x00,0xff,0x00,0x0a,0x0a,
	0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x00,0xff,0xff,0x00,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,
	0x0a,0x00,0xff,0xff,0xff,0x00,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x00,0xff,0xff,0xff,
	0xff,0x00,0x0a,0x0a,0x0a,0x0a,0x0a,0x00,0xff,0xff,0xff,0xff,0xff,0x00,0x0a,0x0a,
	0x0a,0x0a,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x0a,0x0a,0x0a,0x00,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0x00,0x0a,0x0a,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0x00,0x0a,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0xff,
	0xff,0xff,0xff,0xff,0x00,0x0a,0x0a,0x0a,0x0a,0x00,0xff,0x00,0x0a,0x00,0xff,0xff,
	0x00,0x0a,0x0a,0x0a,0x00,0x00,0x0a,0x0a,0x00,0xff,0xff,0x00,0x0a,0x0a,0x0a,0x0a,
	0x0a,0x0a,0x0a,0x0a,0x00,0xff,0xff,0x00,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x00,
	0xff,0xff,0x00,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x0a,0x00,0xff,0xff,0x00,0x0a,
   };

	static char DiskIcon[] = 
   {
	15, 00,
	15, 00,
	00, 00,
	00, 00,
	32, 00,
	0x34,0x34,0x07,0x07,0x07,0x07,0x07,0x07,0x37,0x37,0x07,0x34,0x34,0x34,0x34,0x34,
	0x34,0x23,0x07,0x07,0x07,0x07,0x07,0x2b,0x2b,0x07,0x34,0x01,0x01,0x34,0x34,0x34,
	0x07,0x24,0x07,0x07,0x07,0x07,0x2c,0x2c,0x07,0x34,0x01,0x01,0x34,0x34,0x34,0x07,
	0x07,0x25,0x07,0x07,0x07,0x2d,0x2d,0x07,0x34,0x34,0x34,0x34,0x34,0x34,0x0d,0x07,
	0x07,0x26,0x07,0x07,0x07,0x07,0x07,0x34,0x34,0x34,0x34,0x34,0x34,0x07,0x0f,0x07,
	0x07,0x07,0x07,0x07,0x07,0x07,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,
	0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,
	0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,
	0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,
	0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x06,0x06,0x06,0x06,0x06,0x06,
	0x06,0x06,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x06,0x34,0x34,0x06,0x05,0x05,0x05,
	0x06,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x05,0x34,0x34,0x05,0x05,0x05,0x05,0x06,
	0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x05,0x34,0x34,0x05,0x05,0x05,0x05,0x06,0x34,
	0x34,0x34,0x34,0x34,0x34,0x34,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x34,0x34,
	0x34
   };

	static char ARMWriteIcon[] = 
   {
	15, 00,
	15, 00,
	13, 00,
	13, 00,
	32, 00,
	0x0f,0x0f,0x0e,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x0e,
	0x2a,0x2b,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x29,
	0x26,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x23,
	0x26,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x23,
	0x26,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x23,
	0x26,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x23,
	0x26,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x23,
	0x26,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x23,
	0x26,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x23,
	0x26,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x23,
	0x26,0x25,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x23,
	0x16,0x15,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x19,
	0x16,0x15,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x19,
	0x19,0x03,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x02,
	0x00
   };

	static char ARMReadIcon[] =
   {
	15, 00,
	12, 00,
	00, 00,
	00, 00,
	32, 00,
	0x20,0x20,0x20,0x20,0x07,0x07,0x07,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x07,0x03,0x03,0x07,0x07,0x07,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x05,0x01,0x07,0x07,0x03,0x06,0x06,0x05,0x20,0x20,
	0x20,0x20,0x20,0x07,0x07,0x03,0x04,0x03,0x03,0x07,0x06,0x05,0x05,0x06,
	0x06,0x07,0x07,0x07,0x07,0x07,0x04,0x01,0x06,0x07,0x03,0x05,0x05,0x06,
	0x06,0x03,0x03,0x07,0x07,0x07,0x07,0x07,0x07,0x06,0x06,0x05,0x05,0x06,
	0x07,0x07,0x07,0x07,0x03,0x07,0x04,0x05,0x07,0x07,0x06,0x05,0x05,0x06,
	0x03,0x03,0x03,0x07,0x07,0x07,0x06,0x01,0x04,0x04,0x04,0x06,0x05,0x06,
	0x06,0x07,0x07,0x07,0x03,0x07,0x06,0x01,0x20,0x01,0x01,0x01,0x04,0x04,
	0x04,0x04,0x06,0x07,0x07,0x07,0x06,0x01,0x20,0x20,0x20,0x20,0x20,0x01,
	0x01,0x01,0x04,0x04,0x06,0x07,0x06,0x01,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x01,0x01,0x04,0x04,0x05,0x01,0x20,0x20,0x20,0x20,0x20,0x20,
	0x20,0x20,0x20,0x20,0x01,0x01,0x01,0x01,0x20,0x20,0x20,0x20
   };
	static char* cursors[] =
   {
	PrevIcon,
	DiskIcon,
	ARMWriteIcon,
	ARMReadIcon,
	};


	if (	(cursor == ArrowCursor && resourceCursorType) ||
			(cursor == DiskCursor && resourceCursorType & RESCSR_DISKCHANGE) ||
			(cursor == ARMReadCursor || cursor == ARMWriteCursor) &&
			(resourceCursorType & RESCSR_ARMCHANGE))
      {
		if(cursor && !cursorSaved)
         {
			GetCursorData(PrevIcon);
			cursorSaved = 1;				// To prevent overwrite of saved cursor
			}
		if(cursor || cursorSaved)
         {
			SetCursorData(cursors[cursor]);
			if(cursor == ArrowCursor)
				cursorSaved = 0;
			}
		}
	
	if ((cursor == DiskCursor && resourceCursorType & RESCSR_DISKMOVE) ||
			(cursor == ARMReadCursor || cursor == ARMWriteCursor) &&
			resourceCursorType & RESCSR_ARMMOVE)
      {
		RPoint	mousePoint;
		RGetMouse(&mousePoint);
		if (cursor == DiskCursor) 
         {
			if ((mousePoint.h -= 5) < 0)
				mousePoint.h = 310;
			}
      else 
         if (cursor == ARMReadCursor)
            {
				if ((mousePoint.v -= 3) < 0)
					mousePoint.v = 180;
			   }
         else 
            {
				if ((mousePoint.v += 5) > 180)
					mousePoint.v = 5;
			   }
		SetMouse(&mousePoint);
		}
#endif
}


int
FindPatchEntry(
	ubyte	resType,
	uword	resId)
{
	ResPatchEntry far*	entry;

	if (!patches)
		return -1;

	/* patch list is terminated with 0xff in resType */
	for (entry = (ResPatchEntry far *) *patches; entry->resType != '\0'; ++entry)
		if (entry->resType == resType && entry->resId == resId)
			return (int) entry->patchDir;
	return -1;
}

void
InitPatches()
{
	int resType, resId, npatches, dirNum;
	char filename[64];
	register ResPatchEntry far  *entry;
	register ResPatchEntry far  *tEntry;
	DirEntry	fileinfo;

	dirNum = 0;
	npatches = 0;
	while (patchDir[dirNum]) {
		addSlash(patchDir[dirNum]);
		for (resType = RES_BASE; resType < RES_BASE + NRESTYPES; resType++) {
			sprintf(filename,"%s%s", patchDir[dirNum], ResNameMakeWildCard(resType));
			if (firstfile(filename,0,&fileinfo)) {
				do {
					if (strchr(fileinfo.name,'.') &&
							fileinfo.name[0] >= '0' && fileinfo.name[0] <= '9') {
						npatches++;
					}
				} while (nextfile(&fileinfo));
			}
		}
		dirNum++;
	}

	if (!npatches)
		return;

	patches = ResLoad(RES_MEM, (npatches + 1) * sizeof(ResPatchEntry));
	entry = (ResPatchEntry far *) *patches; 

	dirNum = 0;
	while (patchDir[dirNum]) {
		for (resType = RES_BASE; resType < RES_BASE + NRESTYPES; resType++) {
			sprintf(filename,"%s%s", patchDir[dirNum], ResNameMakeWildCard(resType));
			if (firstfile(filename,0,&fileinfo)) {
				do {
					if (strchr(fileinfo.name,'.') &&
							fileinfo.name[0] >= '0' && fileinfo.name[0] <= '9') {
						resId = atoi(fileinfo.name);
						for (tEntry = (ResPatchEntry far *) *patches;
								tEntry != entry; tEntry++) { 
							if (tEntry->resType == (ubyte) resType &&
									tEntry->resId == resId) {
								--npatches;
								break;
							}
						}
						if (tEntry == entry) {
							entry->patchDir = (ubyte) dirNum;
							entry->resType = (ubyte) resType;
							entry->resId = resId;
							++entry;
						}
					} 
				} while (nextfile(&fileinfo));
			}
		}
		dirNum++;
	}
	entry->resType = '\0';
	ReallocateHandle(patches, (npatches + 1) * sizeof(ResPatchEntry));
}

void
MakeName36(uchar resType, char* fname, uint module, uchar noun,
			  uchar verb, uchar cond, uchar sequ)
{
	if (resType == RES_SYNC)
		fname[0] = '#';
	else
		fname[0] = '@';
	ConvBase36(&fname[1],module,3);
	ConvBase36(&fname[4],(uint)noun,2);
	ConvBase36(&fname[6],(uint)verb,2);
	fname[8] = '.';
	ConvBase36(&fname[9],(uint)cond,2);
	ConvBase36(&fname[11],(uint)sequ,1);
	fname[12] = '\0';
}

void
ConvBase36(str, num10, digits)
char	*str;
uint	num10;
int	digits;
{
	int	n, t;

	t = 0;
	if (digits >= 3) {
		str[t++] = GetDigit36(n = num10 / (36*36));
		num10 -= n * (36*36);
	}
	if (digits >= 2) {
		str[t++] = GetDigit36(n = num10 / 36);
		num10 %= 36;
	}
	str[t] = GetDigit36(n = num10);
}

char
GetDigit36(n)
int	n;
{
	if (n <=  9)
		return((char)('0' + n));
	else
		return((char)('A' + n - 10));
}


