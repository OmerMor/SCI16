#if !defined(STDLIB_H)
#define STDLIB_H

#if !defined(TYPES_H)
#include	"types.h"
#endif

int		atoi(strptr str);
strptr	itoa(int num, strptr str, int radix);
strptr	ultoa(ulong num, strptr str, int radix);

#endif
