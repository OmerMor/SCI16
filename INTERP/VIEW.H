/* VIEW.H - View structure definitions */
/* When loaded in the game, the offsets are relative to the "view" */


#ifndef	VIEW_H
#define	VIEW_H

#include	"types.h"
#include "palette.h"

#define	MAXLOOPS	8
#define	MAXCELS	16
#define	MAXDATA	500		/* bytes per cel */
#define	MAXWIDE	20		/* max width of cel */
#define	MAXHIGH	30		/* max height of cel */

#define	NOCOMPRESS 64	/* define for vFlags for a view with no compression*/

/* cel structure */
struct	aCel{
	word		xDim;			/* x dimension of cel */
	word		yDim;			/* y dimension of cel */
	word		xOff;			/* x axis offset */
	word		yOff;			/* y axis offset */
	byte		skipColor;		/* skip color for cel */
	byte		compressType;	
	word		compressRemapCount;
	long		compressSize;
	long		controlSize;
	long		paletteOffset;
	long		dataOff;			/* byte data for the cel */
};

typedef	struct aCel aCel;

/* loop structure */
struct aLoop {
	word		numCels;			/* number of cels in the loop */
	word		loopSpare;		/* spare, used in editor currently */
	word		celOff[MAXCELS];	/* offsets to the cels */
};
typedef	struct aLoop aLoop;

/* view structure */
struct	aView {
	word		loopPtr;
	byte		numLoops;
	byte		vFlags;
                          /* bit7 indicates a 256 color view */
                          /* bit6 indicates that the view is not RLE compressed */
	word		mirrorBits;		/* if set, requires mirror of cel in loop */
	word		equalBits;		/* if set, indicates that loop data is shared */
	word		viewText;		/* null if no text invloved */
	word		loopOff[MAXLOOPS];	/* offsets to the loops */
};
typedef struct aView aView;

/*
struct newView {
	int viewHeaderSize;
	uchar numLoops;
	uchar dummy;
	uchar hIsSplitFlag;
	uchar	dummy2;
	uint	celCount;
	newPalette far * paletteOffset;
	uchar	loopHeaderSize;
	uchar	celHeaderSize;
	void far * animationOffset;

};

typedef struct newView newView;

*/

#endif

