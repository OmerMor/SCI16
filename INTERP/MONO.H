#ifndef MONO_H
#define MONO_H

#include "types.h"

extern bool		xlate;

#ifdef	LINT_ARGS

	extern void		MonoStr(char far *text);
	extern void		DoXlate(void);

#else

	extern void		MonoStr();
	extern void		DoXlate();

#endif
#endif
