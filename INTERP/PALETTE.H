//	palette.h		header file for PAL16.c and PAL256.c 

#if !defined(PALETTE_H)
#define PALETTE_H

#if !defined(TYPES_H)
#include "types.h"
#endif

#define	PAL_CLUT_SIZE	256

typedef struct {		// ! Does not allow for multPals
	uchar		paletteHdSize; // Size of palette headers
	char		sysPalName[9];
	uchar		paletteCount;	// Number of palettes in main palette
	uint		dummy		; 			// Future expansion
	uint		paletteSize;
	char		danPaltitle[9];
	uchar		sysPalNum	; // Tells which single palette in syspal we expect
	uchar		startOffset ;
	uchar		nCycles		; // number of cycling ranges following header
	uint		dummy2;
	uint		nColors;		 // number of "colors" defined in this palette
	uchar		defaultFlag;  // "Default" flag setting
	uchar		type; 			// (0 = each RGB has flag)
								//  (1 = All RGBs share default flag)
								// (2 = Remap type palette)
	long		valid;			// time stamp

} danPalette;

typedef struct {
	unsigned char flags;
	unsigned char r;
	unsigned char g;
	unsigned char b;
} Guns;

// Guns "flags" definitions
#define	PAL_IN_USE		1	// this entry is in use
#define	PAL_NO_MATCH	2	// never match this color when remapping
#define	PAL_NO_PURGE	4	// never overwrite this color when adding a palette 
#define	PAL_NO_REMAP	8	// never remap this value to another color 
#define	PAL_MATCHED		16	// in sys pal, shows someone is sharing it 

typedef struct {
	unsigned char	mapTo[PAL_CLUT_SIZE];
	long			valid;
	Guns			gun[PAL_CLUT_SIZE];
	word			palIntensity[PAL_CLUT_SIZE];
} RPalette;

enum palFunc {
	PALNULLENUM, PALLoad, PALSet, PALReset, PALIntensity, PALMatch, PALCycle,
	PALSave, PALRestore
};

// maximum number of ranges for palette cycling
#define	MAXPALCYCLE	16

// mode values for InsertPalette()
#define	PAL_MATCH	1
#define	PAL_REPLACE	2

#define	BLACK			0
#define	BLUE			PalMatch(0x15, 0x15, 0x77)
#define	GREEN			PalMatch(0x15, 0x77, 0x15)
#define	CYAN			PalMatch(0x15, 0x77, 0x77)
#define	RED			PalMatch(0x77, 0x15, 0x15)
#define	MAGENTA		PalMatch(0x77, 0x15, 0x77)
#define	BROWN			PalMatch(0x77, 0x46, 0x15)
#define	LGREY			PalMatch(0x9f, 0x9f, 0x9f)
#define	GREY			PalMatch(0x5f, 0x5f, 0x5f)
#define	LBLUE			PalMatch(0x26, 0x26, 0xd8)
#define	LGREEN		PalMatch(0x26, 0xd8, 0x26)
#define	LCYAN			PalMatch(0x26, 0xd8, 0xd8)
#define	LRED			PalMatch(0xd8, 0x26, 0x26)
#define	LMAGENTA		PalMatch(0xd8, 0x26, 0xd8)
#define	YELLOW		PalMatch(0xd8, 0xd8, 0x26)
#define	WHITE			255

extern RPalette	sysPalette;
extern uint       remapPercent[2];
extern uint       remapPercentGray[2];
extern uint			palStamp;

void	InitPalette(void);
void	KPalette(word *);
void	SetPalIntensity(RPalette far *, word, word, word);
uint PalMatch(uint r, uint g, uint b);
void InsertPalette(RPalette far *, RPalette far *);
void SubmitPalette(danPalette far * theView);
void CopyNew2OldPalette(RPalette far * destPal, danPalette far * thePal);
void SetResPalette(int);
void RSetPalette(RPalette far *, int);
void RSetDanPalette(Handle);
void RemapByPercent(int);
void RemapToGray(int);
void RemapToPercentGray(int);

#endif
