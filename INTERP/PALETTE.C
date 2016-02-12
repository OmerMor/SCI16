#include "palette.h"
#include	"cels.h"

#include	"types.h"
#include	"restypes.h"
#include	"resource.h"
#include	"math.h"
#include	"intrpt.h"
#include	"kernel.h"
#include "picture.h"
#include	"grtypes.h"
#include	"graph.h"
#include "memmgr.h"
#include "dialog.h"
#include	"errmsg.h"

RPalette sysPalette;
uint      remapPercent[2] = {0,0}; // this is used to setup the remap table for trans
									// views.  it must be updated as palette changes.
uint      remapPercentGray[2] = {0,0}; // the grey percentage 0 to 100 for the remaping of
										 // trans colors to gray components

static int  near	Match(Guns far *, RPalette far *, ulong);
static void near	ResetPaletteFlags(RPalette far *, word, word, ubyte);
static void near	SetPaletteFlags(RPalette far *, word, word, ubyte);

static struct PaletteCycleStruct {
	long	timestamp[MAXPALCYCLE];
	char	start[MAXPALCYCLE];
} palCycleTable;

void CopyNew2OldPalette(RPalette far * destPal, danPalette far * thePal) {

	uchar far * theData;
	int hasFlag;
	int i,j;
	uchar theFlag;


	for(i = 0; i < 256; i++) {
		destPal->gun[i].flags = 0;
		destPal->gun[i].r = 0;
		destPal->gun[i].g = 0;
		destPal->gun[i].b = 0;
	}

	if(!thePal || !thePal->paletteCount) {	// if we have already done this palette
		return;
	}



	switch(thePal->type) {
		case 0:
			hasFlag = TRUE;
			break;
		case 1:
			hasFlag = FALSE;
			theFlag = thePal->defaultFlag;
			break;
		case 2:
			break;
	}

	theData = (uchar far *) &thePal->valid + 4 + 4 * thePal->nCycles;
	j = thePal->startOffset;

	if(j + thePal->nColors > 256) {
		RAlert(E_BAD_PALETTE);
		return;
	}

	for(i = 0; i < thePal->nColors && j < 255; i++) {

		if(hasFlag)
			theFlag = *theData++;

		if(theFlag) {
			destPal->gun[j].flags = theFlag;
			destPal->gun[j].r = *theData;
			destPal->gun[j].g = *(theData + 1);
			destPal->gun[j].b = *(theData + 2);

		}

		theData += 3;	// point to next rgb or flag/rgb
		j++;

	}

}

/*
		This procedure takes each palette color multiplies each of its color components
		by a percentage between 0 and 255 and finds a best match of that new color in
		the system palette. storing the resultent of the search in the remap table of
		the given color index.  thw remap table is then used to remap colors in the
		VMAP when a color index 254 is placed above them.  This is a pseodo translucent
		effect
*/

global void RemapByPercent(int index)
{
	int i;
   int red, green, blue;

		for ( i = 1; i < 236; i++ ) {
//			the remapPercent is limmeted to 255 so multiplying in an unsigned int
//			should be sufficient to avoid overflow (255 * 255) < 2 ** 16
			red = ((uint) sysPalette.gun[i].r * remapPercent[index] / 100U);
			green = ((uint) sysPalette.gun[i].g * remapPercent[index] / 100U);
			blue = ((uint) sysPalette.gun[i].b * remapPercent[index] / 100U);
 //      call remap and make sure the value do not exceed 255
			remapTable[i+index*256] = (uchar) PalMatch(
									red > 255U ? 255U : red,
									green > 255U ? 255U : green,
									blue > 255U ? 255U : blue
                         );
      }

}

 
global void RemapToGray(int index)
{
   int i, lum;

	for ( i = 1; i < 236; i++ ) {
		// use unsined values for mult to avoid green * 151 from overflow.  (if green = 255
      // green * 151 overflows an int but not a uint
		lum = (((uint) sysPalette.gun[i].r * 77U +
				(uint) sysPalette.gun[i].g * 151U +
				(uint) sysPalette.gun[i].b * 28U ) / 256U);
		remapTable[i+index*256] = (uchar) PalMatch(
								(int) sysPalette.gun[i].r - (((int) sysPalette.gun[i].r - lum)
								* (int) remapPercentGray[index] / 100),
								(int) sysPalette.gun[i].g - (((int) sysPalette.gun[i].g - lum) 
								* (int) remapPercentGray[index] / 100),
								(int) sysPalette.gun[i].b - (((int) sysPalette.gun[i].b - lum) 
								* (int) remapPercentGray[index] / 100)
                        );
   }
		
}

global void RemapToPercentGray(int index)
{
	int i;
   uint lum;
   uint red, green, blue;

	for ( i = 1; i < 236; i++ ) {
		lum = (((uint) sysPalette.gun[i].r * 77U +
				(uint) sysPalette.gun[i].g * 151U +
				(uint) sysPalette.gun[i].b * 28U ) / 256U);
		lum = lum * remapPercent[index] / 100U;
		red =	(int) sysPalette.gun[i].r 
               - (((int) sysPalette.gun[i].r - (int) lum)
					* (int)remapPercentGray[index] / 100);
		green = (int) sysPalette.gun[i].g 
               - (((int) sysPalette.gun[i].g - (int)lum)
					* (int)remapPercentGray[index] / 100);
		blue = (int) sysPalette.gun[i].b 
               - (((int) sysPalette.gun[i].b - (int)lum) 
					* (int)remapPercentGray[index] / 100);

		remapTable[i+index*256] = (uchar) PalMatch(
								red > 255U ? 255U : red,
								green > 255U ? 255U : green,
								blue > 255U ? 255U : blue
                        );
   }
			
}

void SubmitPalette(danPalette far * thePal) {


	uchar far * theData;
	int hasFlag;
	int i,j;
	uchar theFlag;
	char palChanged = 0;

	if(!thePal || !thePal->paletteCount)
		return;

	switch(thePal->type) {
		case 0:
			hasFlag = TRUE;
			break;
		case 1:
			hasFlag = FALSE;
			theFlag = thePal->defaultFlag;
			break;
		case 2:
			break;
	}


	theData = (uchar far *) &thePal->valid + 4 + 4 * thePal->nCycles;
	j = thePal->startOffset;

	for(i = 0; i< thePal->nColors && j < 255; i++) {

		if(hasFlag)
			theFlag = *theData++;

		if(theFlag) {
			palChanged = 1;
			sysPalette.gun[j].flags = theFlag;
			sysPalette.gun[j].r = *theData;
			sysPalette.gun[j].g = *(theData + 1);
			sysPalette.gun[j].b = *(theData + 2);

		}

		theData += 3;	// point to next rgb or flag/rgb
		j++;

	}

	if(palChanged) {
		
		if(!picNotValid){
			SetCLUT((RPalette far *) &sysPalette, FALSE);
		}

		thePal->valid = palStamp;
		sysPalette.valid = thePal->valid; // ???
      for (i=REMAPCOLORSTART; i <= REMAPCOLOREND; i++) {
			if (remapOn[i - REMAPCOLORSTART] == REMAPPERCENT)
				RemapByPercent(i - REMAPCOLORSTART);
			else if (remapOn[i - REMAPCOLORSTART] == REMAPGRAY)
				RemapToGray(i - REMAPCOLORSTART);
			else if (remapOn[i - REMAPCOLORSTART] == REMAPPERCENTGRAY)
				RemapToPercentGray(i - REMAPCOLORSTART);
      }
	}
}


global void InitPalette()
{
	int i;
	for (i= 0 ; i < PAL_CLUT_SIZE; i++){
		sysPalette.gun[i].flags = 0;
		sysPalette.gun[i].r = 0;
		sysPalette.gun[i].g = 0;
		sysPalette.gun[i].b = 0;
		sysPalette.palIntensity[i] = 100;
	}
	sysPalette.gun[0].flags = PAL_IN_USE;
	

	/* set up index 255 as white */
	sysPalette.gun[255].r = 255;
	sysPalette.gun[255].g = 255;
	sysPalette.gun[255].b = 255;
	sysPalette.gun[255].flags = PAL_IN_USE;

	SetResPalette(999);
}

/* all SCRIPT palette functions dispatch through KPalette */
global void KPalette(args)
argList;
{
	Guns aGun;
	int count, index, replace;
	char start;
	long ticks;

	switch (arg(1)){
		case PALLoad:		/* gets a palette resource number */
			SetResPalette(arg(2));
			break;
		case PALSet:	/* gets a start, end, & bits to set in sysPalette */
			SetPaletteFlags((RPalette far *) &sysPalette, arg(2), arg(3), (ubyte) arg(4));
			break;
		case PALReset:	/* gets a start, end, & bits to reset in sysPalette */
			ResetPaletteFlags((RPalette far *) &sysPalette, arg(2), arg(3), (ubyte) arg(4));
			break;
		case PALIntensity:	/* gets a start, end and intensity */
			SetPalIntensity((RPalette far *) &sysPalette, arg(2), arg(3), arg(4));
			if (argCount < 5 || arg(5) == FALSE)
				SetCLUT((RPalette far *) &sysPalette, TRUE);
			break;
		case PALMatch:	/* given R/G/B values, returns the index of this color */
			aGun.r = (ubyte) arg(2);
			aGun.g = (ubyte) arg(3);
			aGun.b = (ubyte) arg(4);
			acc = Match((Guns far *) &aGun, (RPalette far *) &sysPalette, -1L);
			break;
		case PALCycle:
			count=-1;
			while((count+=3)<argCount)
			{
				start=(char)arg(count);
				/* Is range passed already in palCycleTable? */
				for(index=0;index<MAXPALCYCLE;index++)
				{
					if(palCycleTable.start[index]==start)
						break;
				}
				/* No--replace oldest range with new range */
				if(palCycleTable.start[index]!=start)
				{
					ticks=0x7fffffff;
					replace=0;
					for(index=0;index<MAXPALCYCLE;index++)
					{
						if(palCycleTable.timestamp[index]<ticks)
						{
							ticks=palCycleTable.timestamp[index];
							replace=index;
						}
					}
					index=replace;
					palCycleTable.start[index]=start;
				}
				/* Is it time to cycle yet? */
				if((sysTicks-palCycleTable.timestamp[index])>=abs(arg(count+2)))
				{
					palCycleTable.timestamp[index]=sysTicks;
					if(arg(count+2)>0)
					{
						/* cycle forward */
						index=start;
						aGun=sysPalette.gun[index];
						for(index++;index<arg(count+1);index++)
							sysPalette.gun[index-1]=sysPalette.gun[index];
						sysPalette.gun[index-1]=aGun;
					}
					else
					{
						/* cycle reverse */
						index=arg(count+1)-1;
						aGun=sysPalette.gun[index];
						for(;index>start;index--)
							sysPalette.gun[index]=sysPalette.gun[index-1];
						sysPalette.gun[index]=aGun;
					}
				}
			}
			/* actually set the palette */
			SetCLUT((RPalette far *) &sysPalette, TRUE);
			break;
		case PALSave:  /* copies systemPalette into a new pointer and returns pointer */
			if(acc = Pseudo(RNewPtr(sizeof(RPalette))))
				GetCLUT((RPalette far *)Native(acc));
			break;
		case PALRestore: /* copies saved palette to systemPalette and disposes pointer */
			if(acc = arg(2))
			{
				RSetPalette((RPalette far *)Native(acc),PAL_REPLACE);
				DisposePtr(Native(acc));
			}
			break;
	}
}

global void SetPalIntensity(palette, first, last, intensity)
RPalette far *palette;
int first, last, intensity;
{
	int i;
	/* protect first and last entry */
	if (first == 0){
		++first;
	}

	if (last > PAL_CLUT_SIZE - 1){
		last = PAL_CLUT_SIZE - 1;
	}

	for (i = first ; i < last ; i++)
		palette->palIntensity[i] = intensity;

}

void
SetResPalette(int number)	/* add this palette to system palette */
{
	Handle hPal;

	if ((hPal = ResLoad(RES_PAL, number))){
		sysPalette.valid = RTickCount();
		RSetDanPalette(hPal);	
	}
}


void
RSetDanPalette(Handle srcPal)	/* add this palette to system palette */
{

	// This routine is a copy of the original RSetPalette designed to
	// accomodate the new file format and merge it with the system palette.

	// InsertPalette((danPalette far *) *srcPal, (RPalette far *) &sysPalette);	
 	if(palVaryOn) 
      {
      /* convert srcPal to old style palette */
		// newPalette always exists when palVaryOn is on
	  	CopyNew2OldPalette((RPalette far *) *newPalette, (danPalette far *) *srcPal);
		InsertPalette((RPalette far *) *newPalette,(RPalette far *) &sysPalette);
	  	InsertPalette((RPalette far *) *newPalette,(RPalette far *) *startPalette);
		sysPalette.valid = RTickCount();
		((danPalette far *) *srcPal)->valid = palStamp;
      }
	else 
      {
		SubmitPalette((danPalette far *) *srcPal);
	   }

}



global void RSetPalette(srcPal, mode)	/* add this palette to system palette */
RPalette far *srcPal;
int mode;
{
	long valid;
	
	/* save to note any change requiring a SetCLUT call */
	valid = sysPalette.valid;
	
	if (mode == PAL_REPLACE || srcPal->valid != sysPalette.valid)
      {
		srcPal->valid = sysPalette.valid;
 		if(palVaryOn) 
         {
	 		InsertPalette((RPalette far *) srcPal,(RPalette far *) *startPalette);
         /* calculate the new palette */
         PaletteUpdate(TRUE,1);
		   }
		else 
			{
			InsertPalette((RPalette far *) srcPal, (RPalette far *) &sysPalette);	
			/* validate the source palette */
			srcPal->valid = sysPalette.valid;
			if (valid != sysPalette.valid && !picNotValid) 
			   	SetCLUT((RPalette far *) &sysPalette, FALSE);
			}
		}
}


void
InsertPalette(RPalette far* srcPal, RPalette far* dstPal)
{
	/* Integrate this palette into the current system palette
	** observing the various rules of combination expressed by
	** the "flags" element of each palette color value & mode.
	*/

	int i;
	char flags;
	long valid;

	valid = RTickCount();

	for (i = 1; i < PAL_CLUT_SIZE - 1; i++){
		/* set remapping index to none by default */
		srcPal->mapTo[i] = (ubyte) i;

		flags = srcPal->gun[i].flags;

		/* only deal with active entries */
		if (!(flags & PAL_IN_USE)){
			continue;
		}

		/* if this color may not be remapped, it
		** must replace current system entry
		*/

		/* if a gun value in dstPal is changed, then it get revalidated */

		/* if this value is in use we must update valid element */
		dstPal->valid = valid;
		dstPal->gun[i] = srcPal->gun[i];
	}
}

static int  near Match(theGun, pal, leastDist)
Guns	far *theGun;
RPalette far *pal;
unsigned long leastDist;
{
	return(FastMatch(pal, theGun->r, theGun->g, theGun->b, PAL_CLUT_SIZE, (uword) leastDist));
}


uint
PalMatch(uint r, uint g, uint b)
{
	Guns	aGun;

	aGun.r = (ubyte) r;
	aGun.g = (ubyte) g;
	aGun.b = (ubyte) b;
	return Match(&aGun, &sysPalette, -1L);
}


static void near ResetPaletteFlags(pal, first, last, flags)
RPalette far *pal;
int first, last;
unsigned char flags;
{
	int i;

	for (i = first ; i < last; i++){
		/* **********************
		**	don't know if this is true yet
		** if (PAL_IN_USE & flags && pal->gun[i].flags & PAL_MATCHED)
		*/
		pal->gun[i].flags &= ~flags;
	}
}

static void near SetPaletteFlags(pal, first, last, flags)
RPalette far *pal;
int first, last;
unsigned char flags;
{
	int i;
	for (i = first ; i < last; i++){
		pal->gun[i].flags |= flags;
	}
}

