//	pmachine.c

#include	"errmsg.h"
#include "language.h"
#include "object.h"
#include "pmachine.h"
#include "sciglobs.h"
#include "selector.h"
#include "start.h"

word	gameStarted	= FALSE;
Obj*	theGame;

#ifndef DEBUG
long	interpVerStamp;
long	gameVerStamp;
// STAMPVER.EXE uses the following particulars to locate the version stamp
long	verStamp = -0x1234;
word	stampFlag1 = 0x5241;
word	stampFlag2 = 0x4552;
word	stampFlag3 = 0x5349;
// end of order-dependent constants
#endif

void
PMachine()
{
	//	Load the class table, allocate the p-machine stack

	Script*	script;
	word		startMethod;

	theGame = 0;

	if (!gameStarted) {
		LoadClassTbl();
		pmCode = (fptr) GetDispatchAddrInHeapFromC;
		restArgs = 0;
		pStack = NeedPtr(PSTACKSIZE);
		pStackEnd = pStack + PSTACKSIZE;
		FillPtr(pStack, 'S');
	}

#if defined(DEBUG)
	ssPtr = &sendStack[-2];
#else
	interpVerStamp = verStamp + VERSTAMPDELTA;
	if (!interpVerStamp || interpVerStamp != gameVerStamp)
	   Panic(E_VER_STAMP_MISMATCH,interpVerStamp,gameVerStamp);
#endif

	scriptHandle = 0;
	GetDispatchAddrInHeapFromC(0, 0, &object, &script);
	theGame = object;
	scriptHandle = script->hunk;
	globalVar = script->vars;

	GetLanguage();

	pmsp = pStack;

	if (!gameStarted) {
		gameStarted = TRUE;
		startMethod = s_play;
	} else
		startMethod = s_replay;

   InvokeMethod(object, startMethod, 0, 0);
}
