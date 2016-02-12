#if !defined(CELS_H)
#define CELS_H

#if !defined(TYPES_H)
#include	"types.h"
#endif

#if !defined(GRTYPES_H)
#include	"grtypes.h"
#endif

#define	REMAPCOLOR	0xfe

extern unsigned char remapTable[256];
extern unsigned int remapOn[2];
extern unsigned char remapDepth[2];
extern char mirrored;

void		AssertViewPalette(Handle,int);
void		DrawCel(Handle view, int loop, int cel, RRect* r, int priority);
void		DrawMagCursor(struct MagCursorStruct  *, struct pseudoCursor  *, int, int);
void		ScaleDrawCel(Handle view, int loop, int cel, RRect* pt, int pri, int realXDim);
void		GetCelRect(Handle, int, int, int, int, int, RRect *);
int		GetNumLoops(Handle);
int		GetNumCels(Handle, int);
word		GetCelWide(Handle, word, word);
word		GetCelHigh(Handle, word, word);
int 		GetCelXOff(Handle, word, word);
int		GetCelYOff(Handle, word, word);
void		SetNewDim(Handle, int, int, int, int, int, int);
Hunkptr	GetCelPointer(Handle, word, word);
void		SetupVMap(word sourcePage);


#define REMAPCOLORSTART 	0xfd
#define REMAPCOLOREND 		0xfe

#endif

