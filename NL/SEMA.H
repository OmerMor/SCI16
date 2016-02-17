#ifndef SEMA_H

#define SEMA_H


#ifndef	TYPES_H
#include "types.h"
#endif

#ifndef	LISTS_H
#include "lists.h"
#endif


#ifdef LINT_ARGS

	extern bool nlsema(nodeptr);

#else

	extern bool nlsema();

#endif

#endif

