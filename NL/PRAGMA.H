#ifndef PRAGMA_H

#define PRAGMA_H


#ifndef	TYPES_H
#include "types.h"
#endif

#ifndef	LISTS_H
#include "lists.h"
#endif


#ifdef LINT_ARGS

	extern bool nlpragma(nodeptr);

#else

	extern bool nlpragma();

#endif


#endif

