// PICTURE.H

#if !defined(PICTURE_H)
#define PICTURE_H

#if !defined(TYPES_H)
#include	"types.h"
#endif

#if !defined(WINDOW_H)
#include	"window.h"
#endif

// show pic styles
#define	HSHUTTER			0
#define	VSHUTTER			1
#define	WIPELEFT			2
#define	WIPERIGHT		3
#define	WIPEUP			4
#define	WIPEDOWN			5
#define	IRISIN			6
#define	IRISOUT			7
#define	DISSOLVE			8
#define	PIXELDISSOLVE	9
#define	FADEOUT			10

// the relative order of these defines is important for ShowPic to work
// correctly.
#define	SCROLLRIGHT	11
#define	SCROLLLEFT	12
#define	SCROLLUP		13
#define	SCROLLDOWN	14

#define	DONTSHOW			0x1000
#define	VMIRROR			0x2000
#define	HMIRROR			0x4000
#define	BLACKOUT			0x8000
// Space for 256 different styles and 8 different flags 
#define	PICFLAGS			0xff00

#define	PDFLAG			0x8000
#define	PDBOFLAG			0x4000

extern word 		picNotValid;
extern RWindow*	picWind;
extern RRect 		picRect;
extern word			showStyle;
extern uword 		showMap;

void 	PriBands(uint far *);
void 	InitPri(word, word);
void	InitPicture(void);
void	ShowPic(word, word);
word	CoordPri(int);
word	PriCoord(int);

#endif

