// PMACHINE.H
// Interfaces to the pmachine.

#if !defined(PMACHINE_H)
#define PMACHINE_H

#if !defined(OBJECT_H)
#include "object.h"
#endif

//Bits in the -info- property.
#define CLASSBIT		0x8000
#define CLONEBIT		0x0001
#define NODISPOSE		0x0002
#define NODISPLAY		0x0004			// don't display in ShowObj()

#define E_BAD_DISPATCH		   0
#define E_BAD_OPCODE		   	1
#define E_BAD_KERNAL		   	2
#define E_LOAD_CLASS		   	3
#define E_NOT_OBJECT		   	4
#define E_BAD_SELECTOR		   5
#define E_CANT_FIXUP		   	6
#define E_ZERO_DIVIDE		   7
#define E_STACK_BLOWN		   8
#define E_ZERO_MODULO		   9
#define E_LEFT_CLONE		   	10
#define E_NO_DONGLE 		   	11
#define E_PACKHANDLE_HEAP		12
#define E_PACKHANDLE_HUNK		13
#define E_PACKHANDLE_FAILURE	14
#define E_ODD_HEAP_RETURNED	15

#define PSTACKSIZE	0x01400

extern Obj*		theGame;
extern bool		gameStarted;
extern fptr		pmCode;
extern memptr	ip;
extern uword*	pmsp;
extern word		acc;
extern uword*	pStack;
extern uword*	pStackEnd;
extern uword*	globalVar;
extern word*	localVar;
extern word*	tempVar;
extern word*	parmVar;
extern word		restArgs;
extern Obj*		object;
extern Handle	scriptHandle;
extern uint		numClasses;
extern bool		defaultES;
extern word		thisScript;
extern memptr	thisIP;

void		PMachine(void);
memptr	GetDispatchAddr(uint, uint);
void		GetDispatchAddrInHeapFromC(word scriptNum, word entry, Obj** obj,
				Script** script);

#endif

