#ifndef VOLLOAD_H
#define VOLLOAD_H

#include "types.h"

#ifdef	LINT_ARGS

	extern Handle	LoadResMap(strptr);
	extern bool		FindDirEntry(ulong *, ubyte, uword, int *);
	extern bool		FindDirEntryMap(ulong *, ubyte, uword, Handle);

#else

	extern Handle	LoadResMap();
	extern bool		FindDirEntry();
	extern bool		FindDirEntryMap();

#endif

#endif

 
