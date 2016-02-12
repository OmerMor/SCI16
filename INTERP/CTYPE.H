#ifndef CTYPE_H
#define CTYPE_H

#if !defined(TYPES_H)
#include	"types.h"
#endif

extern bool	islower(uchar c);
extern bool	isupper(uchar c);
extern char	_tolower(uchar c);
extern char _toupper(uchar c);

/* need defines to avoid Microsoft C defaults
*/
#define	lower(c)		tolower(c)
#define	tolower(c)	_tolower(c)
#define	toupper(c)	_toupper(c)

#define	isalpha(c)	( islower(c) || isupper(c) )
#define	isdigit(c)	( (c) >= '0' && (c) <= '9')

#define  isalnum(c)  isalpha(c) || isdigit(c) 
#define  iscntrl(c)  ((c) <= 0x1f || (c) == 0x7f )
#define  isspace(c)  strchr(" \t\n\r",c)
#define  ispunct(c)  !(isalnum(c) || isspace(c) || iscntrl(c))

#define	iseuro(c)	( (c) >= 0x80 && (c) <= 0xa5 )
#define	iskana(c)	( (c) >= 0xa6 && (c) <= 0xdf )
#define	isgreek(c)	( (c) >= 0xe0 && (c) <= 0xee )

#define iskanji(c)	((unsigned char) (c) >= 0x81 &&\
			 (unsigned char) (c) <= 0x9f ||\
			 (unsigned char) (c) >= 0xe0 &&\
			 (unsigned char) (c) <= 0xfc)
#define iskanji2(c)	((unsigned char) (c) >= 0x40 &&\
			 (unsigned char) (c) <= 0x7e ||\
			 (unsigned char) (c) >= 0x80 &&\
			 (unsigned char) (c) <= 0xfc)

#endif
