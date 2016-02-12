#ifndef MOTION_H
#define MOTION_H

#include "types.h"

#ifdef	LINT_ARGS

	extern void		KSetJump(word *);
	extern void		KBaseSetter(word *);
	extern void		KDirLoop(word *);
	extern void		KInitBresen(word *);
	extern void		KDoBresen(word *);
	extern void		KDoAvoider(void);

#else

	extern void		KSetJump();
	extern void		KBaseSetter();
	extern void		KDirLoop();
	extern void		KInitBresen();
	extern void		KDoBresen();
	extern void		KDoAvoider();

#endif
#endif
