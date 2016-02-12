#ifndef FILELOAD_H
#define FILELOAD_H

#include "types.h"

#ifdef	LINT_ARGS

	extern Handle	DoLoad(ubyte, uint);
	extern void		InitResource(strptr);
	extern int		OpenResInPath(int, uint, strptr);

#else

	extern Handle	DoLoad();
	extern void		InitResource();
	extern int		OpenResInPath();

#endif


#endif

 
