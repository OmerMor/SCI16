#ifndef DEBUG_H
#define DEBUG_H

#include "types.h"
#include "grtypes.h"
#include "window.h"
#include "palette.h"

extern bool		findRet;
extern int		newRoomNum;
extern bool		traceOn;
extern bool		trace;
extern bool		proOn;
extern uword*	lastSp;
extern Handle	proBuffer;
extern bool		isDebugVersion;
extern uint		proIndex;
extern uint		proBufSize;
extern bool		trackResUse;
extern bool		trackHunkUse;

#ifdef	LINT_ARGS

	extern	void		KSetDebug(word *);
	extern	void		KShowSends(word *);
	extern	void		KShowFree(void);
	extern	void		KInspectObj(word *);

   #ifdef	DEBUG
	   extern	void		KProfiler(word *);
   #else
	   extern	void		KProfiler(void);
   #endif

	extern	void		ShowSends(bool);
	extern	void 		WriteResUse(int,int);
	extern	void		SetDebug(bool);
	extern	void		CheckHunkUse(uint);
	extern	void		Debug(Hunkptr, uword *);
	extern	void		ToggleDebug(void);
	extern	void		Resources(void);
	extern	RWindow	*DebugWindow(strptr, strptr, int, int, int, bool);
	extern	RWindow	*SizedWindow(strptr title, strptr data, bool wait);
	extern	void		PError(memptr, uword *, int, uint, uint);
	extern   strptr	ArgNameRead(strptr);
	extern	char		*ReadMsg(int);
		
#else

	extern	void		KSetDebug();
	extern	void		KShowSends();
	extern	void		KShowFree();
	extern	void		KInspectObj();

	extern	void		ShowSends();
	extern	void		WriteResUse();
	extern	void		SetDebug();
	extern	void		CheckHunkUse();
	extern	void		Debug();
	extern	void		ToggleDebug();
	extern	void		Resources();
	extern	RWindow	*SizedWindow();
	extern	RWindow	*DebugWindow();
	extern	void		PError();
	extern   strptr	ArgNameRead(strptr);
	extern	char		*ReadMsg();
	
#endif

#endif
