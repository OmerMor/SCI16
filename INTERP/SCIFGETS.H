#ifndef SCIFGETS_H
#define SCIFGETS_H

#include "types.h"

#ifdef LINT_ARGS
	extern strptr sci_fgets(strptr str, int len, int fd);
#else
	extern strptr sci_fgets();
#endif

#endif
