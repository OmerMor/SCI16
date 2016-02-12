//	start.h

#if !defined(START_H)
#define START_H

#if !defined(TYPES_H)
#include "types.h"
#endif

#define MAX_QUITSTR	256

extern int		argc;
extern char*	argv[];
extern char		bssEnd;
extern char		bssStart;
extern char		commandLine[];
extern word		gameCode;
extern uint		numHandles;
extern strptr	panicStr;
extern word		pspSeg;
extern char		quitStr[];
extern word		stackBase;

void	Start();
void	ExitFromC(char exitCode);
void	onexit(fptr);
void	exit(char);
void	WriteChar(char chr);

#endif
