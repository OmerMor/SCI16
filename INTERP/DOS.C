/* DOS.C
 * Functions which may differ between OSes.
 */

 

#include "dos.h"

#include	"start.h"
#include	"ctype.h"
#include	"types.h"
#include	"resource.h"
#include	"resname.h"
#include	"fileio.h"
#include	"string.h"
#include	"stdlib.h"
#include	"stdio.h"
#include	"debug.h"
#include	"savegame.h"
#include "intrpt.h"
#include "fileio.h"
#include	"errmsg.h"
#include	"pmachine.h"
#include	"sci.h"

//	KCheckFreeSpace functions
enum CheckFreeSpaceFuncs {
	SAVEGAMESIZE,
	FREESPACE,
	ENOUGHSPACETOSAVE
};

global strptr CleanDir(dir)
strptr	dir;
/* Make sure that the last character of the directory is NOT a '/' or '\'.
 */
{
	strptr	dp;

	dp = &dir[strlen(dir)-1];
	if (*dp == '/' || *dp == '\\')
		*dp = '\0';

	return (dir);
}



global void KCheckFreeSpace(args)
word	*args;
/* 
   If only path argument
      {
      See if there is enough free space on the 
      disk in 'path' to save a game.
      }
   else
      {
      If Arg2 = SAVEGAMESIZE
         {
         Return the amount of space needed to save
         the game in K
         }
      If Arg2 = FREESPACE
         {
         Return the amount of free space in K up 
         to 32Meg
         }
      IF Arg2 = ENOUGHSPACETOSAVE
      	{
      	Return whether there is enough space to save in path
      	}
      }

*/
{
	strptr	path;
	ulong		freeSpace;

	/* Find out how much space there is.
	 */
	path = Native(*(args+1));
	if (*(path+1) == ':')
		freeSpace = RGetFreeSpace(tolower(*path));
	else
		freeSpace = RGetFreeSpace(0);

	if (argCount < 2)
      {
      /* Return boolean, there is enough free space
         on the disk in 'path' to save a game.
      */
	   acc = freeSpace > GetSaveLength();
      }
   else
      {
	   switch (arg(2)) 
         {
		   case SAVEGAMESIZE:
            /* Return the amount of space 
               in K needed to save the game 
            */
	         acc = (int) ((GetSaveLength()+1023)/1024L);
			   break;

		   case FREESPACE:
            /* Return the amount of free space
               in K up to 32Meg
            */
	         freeSpace = freeSpace/1024L;
	         acc = (freeSpace > 32767L)? 32767 : (int) freeSpace; 
			   break;

		   case ENOUGHSPACETOSAVE:
            /* Return boolean, there is enough free space
               on the disk in 'path' to save a game.
            */
	         acc = freeSpace > GetSaveLength();
			   break;
         }
      }
}



global void KValidPath(args)
word	*args;
/* Return TRUE if the passed path is valid, FALSE otherwise.
 * Implementation is to do a firstfile() for the directory, specifying
 * directory only.
 */
{
	strptr	path;
	char		file[65], c;
	DirEntry	dta;

	path = Native(*(args+1));

	strcpy(file, path);
	CleanDir(file);

//	diskIOCritical = FALSE;
	/* if critical error occurs -- don't display fail/retry message */
	diskIOCritical = 5586;

	if (strlen(file) == 0)
		/* Current directory is valid.
		 */
		acc = TRUE;
	else if (file[strlen(file)-1] == ':') {
		/* Current directory on a specified drive is same as validity of drive.
		 */
		c = tolower(file[0]);
		if (acc = existdrive(c)) RGetFreeSpace(c);
		if (criticalError)
			acc = FALSE;
		}
	else if (firstfile(file, F_HIDDEN | F_SYSTEM | F_SUBDIR, &dta))
		/* Check to see if the path is a subdirectory.
		 */
		acc = (dta.atr | F_SUBDIR);
	else
		/* Not valid.
		 */
		acc = FALSE;

	diskIOCritical = TRUE;
}



global void KDeviceInfo(args)
word	*args;
{
	strptr	str;

	str = Native(arg(2));
	switch (arg(1)) {
		case GETDEVICE:
			acc = Pseudo(GetDevice(str, Native(arg(3))));
			break;
		case CURDEVICE:
			acc = Pseudo(GetCurDevice(str));
			break;
		case SAMEDEVICE:
			acc = (strcmp(str, Native(arg(3))) == 0);
			break;
		case DEVREMOVABLE:
			acc = drivecheck(str);
			break;
		case CLOSEDEVICE:
			/* A do-nothing on the IBM, but needed on the Mac and Amiga.
			 */
			break;
		case SAVEDEVICE:
			/* Return letter of writeable device incase of CD-ROM-based game.
			   (Presume same device as where file used to start game)
			 */
			acc = whereDisk;
			break;
		case SAVEDIRMOUNTED:
			/* A do-nothing on the IBM, but needed on the Mac and Amiga.
			 */
			acc = 1; /* assume save disk is mounted by user */
			break;
		case MAKESAVEDIRNAME:
			MakeDirName(str, Native(arg(3)));
			acc = arg(2);
			break;
		case MAKESAVEFILENAME:
			MakeFileName(str, Native(arg(3)), arg(4));
			acc = arg(2);
			break;
		}
}



global strptr GetDevice(path, device)
strptr	path, device;
{
	if (*(path+1) != ':')
		GetCurDevice(device);
	else {
		*device++ = *path++;
		*device++ = *path++;
		*device = '\0';
		}

	return (device);
}



global strptr GetCurDevice(device)
strptr	device;
{
	char	path[65];

	getcwd(path);
	return (GetDevice(path, device));
}


global void ExitThroughDebug()
{
#ifdef DEBUG
	SetDebug(TRUE);
	Debug(ip, pmsp);
#else
	exit(1);
#endif
}

global void GetExecDir(path)
strptr path;
{
#ifdef DEBUG
	int	i;
	char	far *cmdPath;
	word	far *envSeg;
	char	far *envPtr;

	envSeg=(word far *)(((long)pspSeg<<16)+0x2c);
	envPtr=(char far *)((long)*envSeg<<16);
	i=1;
	while(!(envPtr[i-1]==0 && envPtr[i]==0))
		i++;
	cmdPath=&envPtr[i+3];
	i=0;
	while(cmdPath[i]!=0)
	{
		path[i]=cmdPath[i];
		i++;
	}
	while(cmdPath[i]!='\\'&&i>0)
		i--;
	path[i+1]=0;
#else
	*path=0;
#endif
}
