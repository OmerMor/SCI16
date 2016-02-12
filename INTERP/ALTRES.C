//	altres.c		alternate resource memory manager
//					Mark Wilden, January 1991

#include "armasm.h"
#include "config.h"
#include "ems.h"
#include "errmsg.h"
#include "flist.h"
#include "memmgr.h"
#include "resource.h"
#include "start.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

/*	each ARM type must provide at least twice the size of largest possible
	resource so that one resource can be swapped out to it in order to be able
	to swap one resource in from it
*/
#define MinAltResMem	(2L * 64 * 1024)

#include "ems.h"
#include "xms.h"
#include "extmem.h"

ARMType* armTypes[NARMTYPES] = {
	NULL,		/*	standard memory	*/
	&ems,
	&xms,
	&extMem,
	NULL,
};

static void		ChecksumSet(LoadLink far* loadLinkp);
static void		ChecksumCheck(LoadLink far* loadLinkp);
static ubyte	ChecksumMake(LoadLink far* loadLinkp);

void
AltResMemInit()
{
	ARMType **	armp;
	int			nARMs = 0;

	if (useAltResMem) {
		for (armp = armTypes + 1; *armp; armp++)
			if ((*armp)->init())
				/*	see if the ARM can provide a bare minimum	*/
				if (ARMMemAvail(*armp) >= MinAltResMem)
					nARMs++;
				/*	if not, throw it back	*/
				else
					(*armp)->term();

	   if (!nARMs)
	   	useAltResMem = FALSE;
	}
}

ARMHandle
AltResMemAlloc(
	uword			size,
	ARMType**	type)
{
	int	armIndex;

   /*	see if any ARM type has enough free memory	*/
	for (armIndex = 1;
		  armTypes[armIndex] &&
			  (!armTypes[armIndex]->active ||
			   ARMMemAvail(armTypes[armIndex]) < size);
		  armIndex++
		 )
		;
	
	if (!armTypes[armIndex])
		return NO_MEMORY;
	
	*type = armTypes[armIndex];	
	return ARMAlloc(*type, size);
}

Handle
AltResMemRead(
	LoadLink far**	loadLinkpp)
{
	Handle	data;

	SetResCursor(ARMReadCursor);

   /* temp lock because we don't want to purge it */
   (*loadLinkpp)->lock = LOCKED;

   if (data = GetResHandle((*loadLinkpp)->size)) {
   	if ((ARMRead(armTypes[(*loadLinkpp)->altResMem],
   			(*loadLinkpp)->lData.armHandle, 0, (*loadLinkpp)->size, *data)))
			Panic(E_ARM_READ);
		ARMFree(armTypes[(*loadLinkpp)->altResMem],
			(*loadLinkpp)->lData.armHandle);
      (*loadLinkpp)->lData.data = data;
		if (!checkingLoadLinks)
			ChecksumCheck(*loadLinkpp);
      (*loadLinkpp)->altResMem	= 0;
   }
   (*loadLinkpp)->lock = UNLOCKED;
	SetResCursor(ArrowCursor);
   return data;
}

bool
AltResMemWrite(
	LoadLink far**	loadLinkpp)
{
	LoadLink far**	purgee;
   ARMHandle		armHandle;
   ubyte				armIndex;
   
   SetResCursor(ARMWriteCursor);
   
   /*	see if any ARM type has enough free memory	*/
	for (armIndex = 1;
		  armTypes[armIndex] &&
			  (!armTypes[armIndex]->active ||
			   ARMMemAvail(armTypes[armIndex]) < (*loadLinkpp)->size);
		  armIndex++
		 )
		;
	
	/*	if not, purge from the last ARM type in the load list until there is	*/
	if (!armTypes[armIndex]) {

		/*	find the last ARM type in the load list */
      for (purgee = (LoadLink far * *) Native(FLastNode(&loadList));
      	  purgee && (!(*purgee)->altResMem || (*purgee)->lock);
			  purgee = (LoadLink far * *) Native(FPrevNode(Pseudo(purgee)))
	       )
			;

		if (!purgee)
			return FALSE;
		
		armIndex = (*purgee)->altResMem;
		
		/*	purge from the ARM type until there's enough free memory on it	*/
		while (1) {
	   	ARMFree(armTypes[armIndex], (*purgee)->lData.armHandle);
	      FDeleteNode(&loadList, Pseudo(purgee));
	      DisposeHandle((Handle) purgee);

			/* enough memory now?	*/
			if (ARMMemAvail(armTypes[armIndex]) >= (*loadLinkpp)->size)
				break;

			/*	nope--find the next sucker	*/
	      for (purgee = (LoadLink far * *) Native(FLastNode(&loadList));
		      	  purgee &&
		      	  ((*purgee)->altResMem != armIndex || (*purgee)->lock);
				  purgee = (LoadLink far * *) Native(FPrevNode(Pseudo(purgee)))
		       )
				;
			if (!purgee)
				return FALSE;
		}
   }

   /*	got here -> armIndex is an ARM type that has enough free memory	*/

	if (!checkingLoadLinks)
		ChecksumSet(*loadLinkpp);

   armHandle = ARMAlloc(armTypes[armIndex], (*loadLinkpp)->size);
	if (armHandle < 0)
	 	Panic(E_ARM_HANDLE);
   if (ARMWrite(armTypes[armIndex], armHandle, 0, (*loadLinkpp)->size,
   		*((*loadLinkpp)->lData.data)) < 0)
	 	Panic(E_ARM_HANDLE);
	
   DisposeHandle((*loadLinkpp)->lData.data);
   (*loadLinkpp)->altResMem = armIndex;
 
   (*loadLinkpp)->lData.armHandle = armHandle;
   
   SetResCursor(ArrowCursor);
   return TRUE;
}

void
AltResMemFree(
	LoadLink far**	loadLinkpp)
{
	ARMFree(armTypes[(*loadLinkpp)->altResMem], (*loadLinkpp)->lData.armHandle);
}

char
AltResMemDebugPrefix(
	LoadLink far**	loadLinkpp)
{
	return armTypes[(*loadLinkpp)->altResMem]->debugPrefix;
}

void
AltResMemDebugSummary(
	strptr	where)
{
	LoadLink far**	loadLink;
	ulong				used;
	ulong				slack;
	ubyte				i;
	ulong				memFree;
	int				pageSize;
	
	for (i = 1; armTypes[i]; i++) {

		if (!armTypes[i]->active)
			continue;

		used = slack = 0;
		memFree	= ARMMemAvail(armTypes[i]);
		pageSize	= armTypes[i]->pageSize;

		for (loadLink = (LoadLink far * *) Native(FFirstNode(&loadList));
			  loadLink;
			  loadLink = (LoadLink far * *) Native(FNextNode(Pseudo(loadLink)))
			 )
			if ((*loadLink)->altResMem == i) {
				used	+= (*loadLink)->size;
				slack += pageSize - (*loadLink)->size % pageSize;
			}
	
		sprintf(where, "%c%4uK %s:  %uK used  %uK slack  %uK free\r",
			armTypes[i]->debugPrefix,
			(uword) ((used + slack + memFree) >> 10),
			armTypes[i]->name,
			(uword) (used >> 10),
			(uword) (slack >> 10),
			(uword) (memFree >> 10)
		);
		where += strlen(where);
	}
}

void
AltResMemDebugKey(
	strptr	where)
{
	int	i;
	
	for (i = 1; armTypes[i]; i++) {
		sprintf(where, "%c %s    ", armTypes[i]->debugPrefix, armTypes[i]->name);
		where += strlen(where);
	}
}

void
AltResMemTerm()
{
	ARMType**	armp;

	for (armp = armTypes + 1; *armp; armp++)
		if ((*armp)->active)
			(*armp)->term();
}

static void
ChecksumSet(
	LoadLink far*	loadLinkp)
{
   loadLinkp->checkSum = ChecksumMake(loadLinkp);
}

static void
ChecksumCheck(
	LoadLink far* loadLinkp)
{
   if (loadLinkp->checkSum != ChecksumMake(loadLinkp)) {
   	RAlert(E_ARM_CHKSUM, loadLinkp->type, loadLinkp->id,loadLinkp->altResMem);
   	exit(1);
   }
}

static ubyte
ChecksumMake(
	LoadLink far*	loadLinkp)
{
	ubyte			checkSum;
	int			i;
	uchar far*	cp = *loadLinkp->lData.data;

   for (i = 0, checkSum = 0;
        i < loadLinkp->size;
        i++, checkSum += *cp++)
   	;
   return checkSum;
}

/////////////////////////////////////////////////////////////////////////////

static word	emsHandle;

void far*
Get32KEMS(void)
{
	//	allocate 32K of EMS memory and map it into the top two pages of
	//	the page frame for unrestricted use

	//	make sure EMS is out there, and that there's at least 32K
	if (EMSDetect() < 2)
		return (void far*) 0;
		
	//	grab the 32K
	if ((emsHandle = EMSAlloc(2)) == NO_MEMORY)
		return (void far*) 0;
	onexit(Free32KEMS);
		
	//	map them into the top of the page frame (and leave them there)
	if (EMSMapPage(2, 0, emsHandle))
		return (void far*) 0;
	if (EMSMapPage(3, 1, emsHandle))
		return (void far*) 0;
		
	//	return a pointer to them
	return (char far*) EMSGetPageFrame() + 32 * 1024;
}

void
Free32KEMS(void)
{
	if (emsHandle) {
		EMSFree(emsHandle);
		emsHandle = 0;
	}
}
