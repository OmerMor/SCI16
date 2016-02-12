/* FILELOAD - Code to load resources from discrete files */

#include "fileload.h"

#include	"config.h"
#include "debug.h"
#include	"event.h"
#include	"fileio.h"
#include	"memmgr.h"
#include	"resource.h"
#include	"resname.h"
#include "start.h"
#include	"errmsg.h"
#include "audio.h"
#include	"sync.h"
#include "armasm.h"

#include "io.h"
#include "stdio.h"
#include "stdlib.h"
#include	"string.h"
#include "dos.h"

global void
InitResource(where)	/* init global resource list */
strptr	where;
{
	InitList(&loadList);
	if (!ReadConfigFile(where, "where"))
		Panic(E_NO_WHERE);
}

global bool
ResCheck(resType, resId)	/* determine if this resource is locateable */
ubyte resType;
uint  resId;
{
	word		fd;
	char		pathName[64];

	pathName[0] = '\0';
	if ((fd = ROpenResFile(resType, resId, pathName)) != CLOSED)
		close(fd);
	else if ((resType != RES_AUDIO && resType != RES_WAVE) ||
				FindAudEntry(resId) == -1L)
		return(FALSE);
	return(TRUE);
}


global bool
ResCheck36(resType, module, noun, verb, cond, sequ)
ubyte resType;
uint  module;
uchar noun, verb, cond, sequ;
{
	uint		dumy;
	char		pathName[64];

	MakeName36(resType, pathName, module, noun, verb, cond, sequ);

	if (resType == RES_AUDIO36) {
		if (FindAud36Entry(module, noun, verb, cond, sequ) != -1L)
			return(TRUE);
		if ((dumy = ROpenResFile(RES_AUDIO, 0, pathName)) == (uint) CLOSED)
			return(FALSE);
		close(dumy);
		return(TRUE);
	}

	if (resType == RES_SYNC36) {
		if (FindSync36Entry(module, noun, verb, cond, sequ, &dumy) != -1L)
			return(TRUE);
		if ((dumy = ROpenResFile(RES_SYNC, 0, pathName)) == (uint) CLOSED)
			return(FALSE);
		close(dumy);
		return(TRUE);
	}

	return(FALSE);
}


global Handle
DoLoad(resType, resId)	/* allocate memory and load this resource */
ubyte resType;
uint  resId;
{
	ubyte		type;
	Handle	base;
	word		fd;
	uword		length;
	long		seek;
	

/* default return pointer to NULL */
	base = 0;	

/* open the file */
	fd = OpenResInPath(resType, resId, (strptr) NULL);
	if (fd == -1)
		return (0);
	length = (uword) lseek(fd, 0L, LSEEK_END);

	AudioPause(resType, resId);

/* rewind to beginning of file to confirm data */
	lseek(fd, 0L, LSEEK_BEG);
	read(fd, &type, 1);

	if (resType != type && (resType != RES_XLATE || type != RES_MSG)) {
		close(fd);
		RAlert(E_WRONG_RESRC, resType, resId);
		ExitThroughDebug();
	}


	switch(resType) {

		case RES_PIC:
			lseek(fd,3L,LSEEK_BEG);
			read(fd,&type,1);
			seek = (long) type + 22L;  // current size of picb header
			break;

		case RES_VIEW:
			lseek(fd,3L,LSEEK_BEG);
			read(fd,&type,1);
			seek = (long) type + 22L;	// current size of viewb header
			break;

		case RES_PAL:
			lseek(fd,3L,LSEEK_BEG);
			read(fd,&type,1);
			seek = (long) type;	// bypass count byte + count
			break;

		default:
			lseek(fd,1L,LSEEK_BEG);
			read(fd,&type,1);
			seek = (long) type;
			break;
	}


/* read length of header and seek to start of data */
	lseek(fd, seek, LSEEK_CUR);
//	length -= (uword) tell(fd);

 	if((resType == RES_PATCH) && (resId == 10)) 	{
		base = GetHandle();
	 	*base = Get32KEMS();			  
	}
	else 	{
/* try for memory of this size but don't exceed */					
		if (!(base = GetResHandle(length))) {
			close(fd);
			AudioResume();
			return(0);
		}
	}

/* read file into this memory */
	readfar(fd, base, length);
	close(fd);

 	AudioResume();
/* return pointer/handle or 0 for error */
	return(base);
}

global int
OpenResInPath(resType, resId, name)
uint  resId;
int	resType;
strptr	name;
/* Search a path for a file.  If present, open it and return the file
 * descriptor, else return -1.  The path searched is specified by resType.
 * If name is zero, use the standard resource naming conventions, e.g.
 * view.012 or 12.v56.  If name is nonzero, it is the name of the file to open.
 * If we're looking for a 256-color view, try a 16-color view if no luck
 */
{
	char		fileName[80];
	int		fd;

/*	OpenResFileInPath copies the full name of the file it finds to its name
	parameter, so pass it a string large enough to write to
*/
	if (!name)
		*fileName = '\0';
	else 
		strcpy(fileName, name);

	fd = ROpenResFile(resType, resId, fileName);
	if (fd != -1)
		return fd;

	if (RAlert(E_LOAD_ERROR, ResNameMake(resType, resId)))
		return -1;
	ExitThroughDebug();
   return 0;
}
