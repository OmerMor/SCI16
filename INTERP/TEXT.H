#ifndef	TEXT_H

#define	TEXT_H

#include "types.h"    /* for typedef strptr */
#include "grtypes.h"  /* for typedef RRect */

/* text justification */
#define TEJUSTLEFT	0
#define TEJUSTCENTER 1
#define TEJUSTRIGHT -1


/* Defines for text strings with special control codes (fonts, colors, etc.) */
#define	MAXTEXTFONTS	100
#define	MAXTEXTCOLORS	100
#define	CTRL_CHAR		'|'

extern uchar	textColors[];
extern uint		textFonts[];
extern uchar	lastTextColor;
extern uchar	lastTextFont;
extern word*	newRect;
extern word		rectIndex;

#ifdef	LINT_ARGS

	extern void		KTextColors(word *);
	extern void		KTextFonts(word *);
	extern void		RTextSize(RRect *, strptr, word, word);
	extern int		GetLongest(strptr *, int, int);
	extern int		GetHighest(strptr, int, int);
	extern word		*RTextBox(strptr, int, RRect *, word, word);
	extern int		RStringWidth(strptr);
	extern void		DrawString(strptr);
	extern void		ShowString(strptr);
	extern void		RDrawText(strptr, int, int, int, int);
	extern void		ShowText(strptr, int, int, int, int);

#else

	extern void		KTextColors();
	extern void		KTextFonts();
	extern void		RTextSize();
	extern int		GetLongest();
	extern int		GetHighest();
	extern word		*RTextBox();
	extern int		RStringWidth();
	extern void		DrawString();
	extern void		ShowString();
	extern void		RDrawText();
	extern void		ShowText();

#endif


#endif

