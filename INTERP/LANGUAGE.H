/*	language.h		3/7/91
*/

#ifndef	LANGUAGE_H
#define	LANGUAGE_H

#include "types.h"

#ifdef LINT_ARGS
	extern	int    GetLanguage(void);
	extern	void	Clearkanjimode(void);
#else
	extern	int    GetLanguage();
	extern	void	Clearkanjimode();
#endif

extern int language;
extern int kanjimode;

#define LANGSEPARATOR "\r----------\r"

/*	Country codes per the phone book */
#define	ENGLISH		1
#define	JAPANESE		81
#define	GERMAN		49
#define	FRENCH		33
#define	SPANISH		34
#define	ITALIAN		39
#define	PORTUGUESE	351

#endif /* ifndef LANGUAGE_H */
