#ifndef VOLLOAD_H
#define VOLLOAD_H

#include "types.h"

#ifdef	LINT_ARGS

	extern Handle	LoadResMap(strptr);
	extern bool		FindDirEntry(ulong *, ubyte, uword, int *);
	extern bool		FindDirEntryMap(ulong *, ubyte, uword, Handle);
	extern Handle	DoLoad(ubyte, uint);
	extern void		InitResource(strptr);

#else

	extern Handle	LoadResMap();
	extern bool		FindDirEntry();
	extern bool		FindDirEntryMap();
	extern Handle	DoLoad();
	extern void		InitResource();

#endif

#endif

 
