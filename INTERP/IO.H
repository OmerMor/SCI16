#ifndef	IO_H
#define	IO_H

/*to get ulong def*/
#include "types.h"

#define	LSEEK_BEG	0
#define	LSEEK_CUR	1
#define	LSEEK_END	2

#define	CLOSED		-1

ulong		lseek(int handle, ulong offset, size_t mode);
int		creat(strptr name, int mode); /*stdioasm.s*/
int		open(strptr name, int mode); /*stdioasm.s*/
size_t	read(int handle, void* buffer, size_t length); /*stdioasm.s*/
size_t	write(int handle, void* buffer, size_t length); /*stdioasm.s*/
int		close(int handle); /*stdioasm.s*/

#endif
