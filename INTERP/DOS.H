#ifndef DOS_H
#define DOS_H

#include "types.h"

#ifdef	LINT_ARGS

	extern strptr	CleanDir(strptr dir);
	extern void		KCheckFreeSpace(word *args);
	extern void		KValidPath(word *args);
	extern void		KDeviceInfo(word *args);
	extern void		ExitThroughDebug(void);
	extern strptr	GetDevice(strptr path, strptr device);
	extern strptr	GetCurDevice(strptr device);
	extern void		GetExecDir(strptr);

#else

	extern strptr	CleanDir();
	extern void		KCheckFreeSpace();
	extern void		KValidPath();
	extern void		KDeviceInfo();
	extern void		ExitThroughDebug();
	extern strptr	GetDevice();
	extern strptr	GetCurDevice();
	extern void		GetExecDir();

#endif

/* DOS function calls */
#define dosResetDisk		0x0d
#define dosSetDrive   	0x0e
#define dosEquipCfg   	0x11
#define dosCurDrive		0x19
#define dosSetDTA	   	0x1a
#define dosSetVector		0x25
#define dosGetVector		0x35
#define dosFreeSpace		0x36
#define dosCreat	      0x3c
#define dosOpen	      0x3d
#define dosClose	      0x3e
#define dosRead	      0x3f
#define dosWrite	      0x40
#define dosUnlink      	0x41
#define dosLSeek	      0x42
#define dosDup		   	0x45
#define dosCurDir	   	0x47
#define dosFirst	      0x4e
#define dosNext		   0x4f
#define dosFileDT	   	0x57

#ifndef _FIND_T_DEFINED

struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
    };

#define _FIND_T_DEFINED

#endif

/* File attribute constants */

#define _A_NORMAL       0x00    /* Normal file - No read/write restrictions */
#define _A_RDONLY       0x01    /* Read only file */
#define _A_HIDDEN       0x02    /* Hidden file */
#define _A_SYSTEM       0x04    /* System file */
#define _A_VOLID        0x08    /* Volume ID file */
#define _A_SUBDIR       0x10    /* Subdirectory */
#define _A_ARCH         0x20    /* Archive file */

/* macros to break MS C "far" pointers into their segment and offset
 * components
 */

#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))



#endif
