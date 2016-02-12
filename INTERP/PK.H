#if !defined(PK_H)
#define PK_H

#if !defined(TYPES_H)
#include	"types.h"
#endif

extern Handle	dcompBuffer;

void		pkExplode(int fd, uchar far *dst, unsigned length);
void		pkImplode(int fd, uchar far *src, unsigned length);
uint		pkImplode2Mem(uchar far *dst, uchar far *src, unsigned length);

#endif
