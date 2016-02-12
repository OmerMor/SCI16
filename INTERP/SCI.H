// SCI.H

#if !defined(SCI_H)
#define SCI_H

#if !defined(SETJMP_H)
#include "setjmp.h"
#endif

extern jmp_buf	restartBuf;
extern char		whereDisk;
extern char		where[];

void	main(int argc, char* argv[]);

#endif
