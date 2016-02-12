#ifndef RESTART_H
#define RESTART_H

#include "types.h"

extern	int		gameRestarted;

#ifdef	LINT_ARGS

	extern void		KRestartGame(void);
	extern void		KGameIsRestarting(word *);

#else

	extern void		KRestartGame();
	extern void		KGameIsRestarting();

#endif
#endif
