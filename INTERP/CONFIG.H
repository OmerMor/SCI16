#if !defined(CONFIG_H)
#define CONFIG_H

#if !defined(TYPES_H)
#include "types.h"
#endif

extern strptr	videoDriver;
extern strptr	soundDriver;
extern strptr	kbdDriver;
extern strptr	joyDriver;
extern strptr	audioDriver;
extern strptr	movieDir;
extern strptr	patchDir[];
extern bool		useMouse;
extern int		useDiskAudio;
extern uint		audBuffKSize;
extern int		audioDMA;
extern int		audioIRQ;

extern uint		minHunkSize;
extern uint		maxHunkUsed;

bool	ReadConfigFile(strptr name, strptr defaultName);

#endif
