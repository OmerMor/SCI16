#ifndef STDIO_H
#define STDIO_H

#include	"types.h"

#ifndef STDARG_H
#include	"stdarg.h"
#endif

/* include io.h to make up for the original, incorrect inclusion
   in this file of declarations for creat, open, close, read and write.
   When there is some spare time, all users of these functions should
   be made to include io.h so that it is no longer required here.
   --Pablo Ghenis
*/
#include	"io.h"

#ifndef SCI
	/* per Microsoft */
	#define  FILE    struct _iobuf
#endif

extern void		puts(strptr str);
extern int		sprintf(char *,char *, ...);
extern int		vsprintf(char *,char *, va_list);

#ifndef SCI
	extern char *	fgets(char *, int, FILE *);
#endif

#endif
