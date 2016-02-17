/* Project: SCI Interpreter
 *
 * Module:	string.h
 *
 * Author:	Jeff Stephenson
 *
 * Defines for string functions.
 */

#ifndef	STRING_H

#define	STRING_H

#ifndef	TYPES_H
#include	"types.h"
#endif

#ifdef	LINT_ARGS

uint		strlen(strptr str);
strptr	strcpy(strptr dest, strptr src);
strptr	strcat(strptr dest, strptr src);
int		strcmp(strptr str1, strptr str2);
strptr	strlwr(strptr str);

#else

uint		strlen();
strptr	strcpy();
strptr	strcat();
int		strcmp();
strptr	strlwr();

#endif

#define	islower(c)	( (c) >= 'a' && (c) <= 'z' )
#define	isupper(c)	( (c) >= 'A' && (c) <= 'Z' )
#define	isalpha(c)	( islower(c) || isupper(c) )
#define	isdigit(c)	( (c) >= '0' && (c) <= '9')

#define  isalnum(c)  isalpha(c) || isdigit(c) 
#define  isspace(c)  ( (c)==' ' || (c)=='\t' || (c)=='\n' || (c)=='\r')
#define  ispunct(c)  !(isalnum(c) || isspace(c))

#define 	_tolower(c)	(char) ( (c)-'A'+'a' )
#define 	_toupper(c)	(char) ( (c)-'a'+'A' )

#define 	toupper(c)	(char) ( (islower(c)) ? _toupper(c) : (c) )
#define 	tolower(c)	(char) ( (isupper(c)) ? _tolower(c) : (c) )


#endif

