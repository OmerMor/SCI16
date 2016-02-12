//	pmachine.c

#include "dongle.h"
#include "language.h"
#include "object.h"
#include "pmachine.h"
#include "sciglobs.h"
#include "selector.h"
#include "start.h"

// This code is game specific and must match global number g_gameCode
#if !defined(GAMECODE)
#define GAMECODE	1234
#endif

word	gameCode = GAMECODE;
word	gameStarted	= FALSE;
Obj*	theGame;

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
#endif

	scriptHandle = 0;
	GetDispatchAddrInHeapFromC(0, 0, &object, &script);
	theGame = object;
	scriptHandle = script->hunk;
	globalVar = script->vars;

	if (globalVar[g_gameCode] != gameCode)
		noDongle = dongleCount;

	GetLanguage();

	pmsp = pStack;

	if (!gameStarted) {
		gameStarted = TRUE;
		startMethod = s_play;
	} else
		startMethod = s_replay;

   InvokeMethod(object, startMethod, 0, 0);
}
