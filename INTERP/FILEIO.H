/* FILEIO
 */



#ifndef	FILEIO_H

#define	FILEIO_H

#include	"types.h"
#include	"kernel.h"
#include	"memmgr.h"

#define	O_RDONLY	(int) 0x0000
#define	O_WRONLY	(int) 0x0001
#define	O_RDWR	(int) 0x0002
#define	O_APPEND	(int) 0x0008	/* writes done at eof */

#define	O_CREAT	(int) 0x0100	/* create and open file */
#define	O_TRUNC	(int) 0x0200	/* open with truncation */
#define	O_EXCL	(int) 0x0400	/* exclusive open */

/* Definitions for various file opening modes.
 */
#define	F_APPEND		0
#define	F_READ		1
#define	F_TRUNC		2

typedef	struct	dirEntry {
	ubyte		reserved[21];
	ubyte		atr;
	uint		time;
	uint		date;
	ulong		size;
	char		name[13];
	}		DirEntry;


/* Attribute bits for 'atr'.
*/
#define	F_READONLY	0x01
#define	F_HIDDEN		0x02
#define	F_SYSTEM		0x04
#define	F_LABEL		0x08
#define	F_SUBDIR		0x10
#define	F_DIRTY		0x20


/* Function codes for DeviceInfo call.
 */
enum devFuncs {
	GETDEVICE,
	CURDEVICE,
	SAMEDEVICE,
	DEVREMOVABLE,
	CLOSEDEVICE,
	SAVEDEVICE,
	SAVEDIRMOUNTED,
	MAKESAVEDIRNAME,
	MAKESAVEFILENAME
	};

enum	fileFuncs {
	fileOpen,
	fileClose,
	fileRead,
	fileWrite,
	fileUnlink,
	fileFGets,
	fileFPuts,
	fileSeek,
	fileFindFirst,
	fileFindNext,
	fileExists,
	fileRename,
	fileCopy
};

extern	bool		criticalError;

size_t	readfar(int handle, Handle buffer, size_t length);
uint		ReadDos (int fd,uchar far * buffer,int len);
size_t	WriteDos(int handle, void far* buffer, size_t length);
ulong		filelength(int handle);
int		unlink(strptr name);
bool		firstfile(strptr spec, int atr, DirEntry *dta);
bool		nextfile(DirEntry *dta);
strptr	getcwd(strptr dir);
bool		existdrive(char);
void		ResetDisk(void);
ulong		RGetFreeSpace(char drive);
int		Exists(strptr);
int		drivecheck(strptr);
int		rename(char* oldname, char* newname);

#endif
