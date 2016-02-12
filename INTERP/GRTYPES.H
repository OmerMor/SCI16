#if !defined(GRTYPES_H)
#define	GRTYPES_H

#if !defined(TYPES_H)
#include	"types.h"
#endif

#if !defined(LIST_H)
#include	"list.h"
#endif

// pen transfer modes
#define	SRCOR			0
#define	SRCCOPY		1
#define	srcInvert	2
#define	SRCAND		3

// character style
#define	PLAIN	0
#define	DIM		1
#define	BOLD		2
#define	INVERSE	4

#define	VMAP		1
#define	PMAP		2
#define	CMAP		4

#define	vBLACK	0
#define	vWHITE	255

// Menu/Dialog "Item" definitions

// Item types
#define	dButton		1
#define	dText			2
#define	dEdit			3
#define	dIcon			4
#define	dMenu			5
#define	dSelector	6
#define	dScroller	7

// control states
#define dActive		0x0001
#define dExit			0x0002
#define dBold			0x0004
#define dSelected		0x0008
#define dIconTop		0x0010
#define dFrameRect	0x0020

typedef struct {
	word v;
	word h;
} RPoint;

typedef struct {
	word top;
	word left;
	word bottom;
	word right;
} RRect;

typedef struct {
	Node		pLink;			// standardized LIST node structure
	RPoint	origin;			// offsets local coords for all drawing
	RRect		portRect;		// all drawing clipped to here
	RPoint	pnLoc;			// persistent text output location
	word		fontSize;		// point size of tallest character
	word		fontNum;			// font number of this port
	word		txFace;			// stylistic variations
	word		fgColor;			// on bits color
	word		bkColor;			// off bits (erase) color
	word		pnMode;			// transfer of bits
} RGrafPort;

bool		CInitGraph(strptr);
void		CEndGraph(void);

void		ROpenPort(RGrafPort *);
void		RGetPort(RGrafPort **);
void		RSetPort(RGrafPort *);
void		RSetOrigin(int, int);

void		RMoveTo(int, int);
void		RMove(int, int);
void		PenColor(int);
void		RBackColor(int);
void		RPenMode(int);
void		RTextFace(int);
word		RSetFont(word);
word		GetFont(void);

word		RTextWidth(strptr, int, int, int);
word		RCharWidth(ubyte);
word		CharHeight(ubyte);
word		GetPointSize(void);
void		SetPointSize(word);

Handle	SaveBits(RRect *, uword);
void		RestoreBits(Handle);
void		RInvertRect(RRect *);
void		RPaintRect(RRect *);
void		REraseRect(RRect *);
void		RFillRect(RRect *, uword, int, ...);
uword		OnControl(uword, RRect *);
void		CanBeHere(RRect *, uword);

void		RDrawChar(ubyte);
void		ShowChar(ubyte);
void		DrawLine(int, int, int, int);
void		DrawBrush(int, int, int, int);
void		FillArea(int, int);

void		ShowBits(RRect *, int);
void		InitDisplay(void);
void		EndDisplay(int);
bool		RShowCursor(void);
bool		RHideCursor(void);
bool		RSetCursor(Handle, word, word, word, word);
void		ShakeScreen(word, word);
void		ShiftScreen(word, word, word, word, word);
void		ColorFade(word, word);

bool		CSectRect(RRect *, RRect *, RRect *);
void		ROffsetRect(RRect *, int, int);
void		RInsetRect(RRect *, int, int);
void		MoveRect(RRect *, int, int);

void		DrawPic(Handle, bool, bool);

#endif
