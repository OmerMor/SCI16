#ifndef	MATH_H
#define	MATH_H

#include	"types.h"

#ifdef	LINT_ARGS

	extern uint		sqrt(ulong);
	extern ulong	LCGRandom(void);
	extern uint		abs(int);
	extern ulong	lcgSeed;

#else

	extern uint		sqrt();
	extern ulong	LCGRandom();
	extern uint		abs();
	extern ulong	lcgSeed;

#endif

#endif

