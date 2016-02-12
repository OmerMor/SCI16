#if !defined(SAVEGAME_H)
#define SAVEGAME_H

#if !defined(TYPES_H)
#include "types.h"
#endif

ulong	GetSaveLength(void);
void	MakeFileName(strptr where, strptr name, int num);
void	MakeDirName(strptr where, strptr name);
void	KSaveGame(word *);
void	KRestoreGame(word *);
void	KGetSaveFiles(word *args);
void	KGetSaveDir(void);
void	KCheckSaveGame(word *args);

extern char	saveDir[];

#endif
