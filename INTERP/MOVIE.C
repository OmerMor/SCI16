//                                  MOVIE.C


// Routines for running a "movie" animation using special a picture file


#include	"kernel.h"

/* SCI-specific header files */

#include	"restart.h"
#include	"grtypes.h"
#include	"animate.h"
#include	"cels.h"
#include	"fileio.h"
#include	"graph.h"
#include	"memmgr.h"
#include "object.h"
#include	"resource.h"
#include	"sound.h"
#include	"start.h"
#include "sync.h"
#include	"text.h"
#include "trig.h"
#include	"window.h"
#include "scifgets.h"
#include "config.h"
#include	"errmsg.h"
#include "movie.h"
#include "intrpt.h"
#include "io.h"
#include "stdio.h"
#include "palette.h"
#include "string.h"
#include "view.h"
#include "picture.h"

#define VACANT 0
#define FILLED 1
#define IN_USE 2

char pageStatus[4];
char currentPage;
char movieOn;
long frameRate;
Handle paletteHandle;

static int celNo;
static void MovieServer(void);
static void RemoveMovieServer(void);
static void SetupMovieServer(void);
void SetMoviePalette(void);

struct {
		int	pageOffset;
		char far *	buffer;		// split into segement and offset in vga320.s
		int	dataOffset;
		int	xOffset;
		int 	yOffset;
		int 	xDim;
		int	yDim;
		char  compressType;
	} xCel ;


void RunMovie(int handle, Handle bufferHandle,int speed)
{

	/* Main loop for displaying reading special format cels from a previously
		opened file.  Cels are displayed approximately 10 frames per second
	*/

	int i;
	int done;
	int lastPage;
	unsigned celCount;


	currentPage = 0;
	frameRate = speed;

	celCount = LoadCelCount(handle);
	LoadMoviePalette(handle);

	CopyPage(3 * 0x4000,0);
	ShowPage(3 * 0x4000);		// point to unused page

	// Fill video pages for initial pass
	for(i = 0; i < 4; i++) {

		if(i == 3) {		// Now Filling Page 3 - switch to page 0
			SetMoviePalette();
			ShowPage(0);
		}

		if(i > 0) 
			CopyPage(i * 0x4000,(i - 1) * 0x4000);

		LoadFrame((char) i,handle,bufferHandle);

		pageStatus[i] = FILLED;

	}

	SetupMovieServer();
	movieOn = TRUE;
	currentPage = 0;

	while (i < celCount) {
		if (pageStatus[currentPage] == VACANT) {

			if(currentPage == 0)
				lastPage = 3;
			else
				lastPage = currentPage - 1;

			CopyPage(currentPage * 0x4000,lastPage * 0x4000);

			LoadFrame((char) currentPage,handle,bufferHandle);
			pageStatus[currentPage] = FILLED;
			i++;
			++currentPage;
			if (currentPage > 3)
				currentPage = 0;
		}
	}


	/* wait for all our filled buffers to be displayed */

	do  {

		done = TRUE;
		for (i = 0; i < 4; i++) {
			if (pageStatus[i] == FILLED) {
				done = FALSE;
				break;
			}
		}
	}
	while (!done);


	// We need to copy the last page filled to page 3 so there are no glitches
	// because the next movie (if any) will start by showing page 3)
	if(currentPage == 0)
		currentPage = 3;
	else
		currentPage--;

	movieOn = FALSE;
	RemoveMovieServer();

	for(i = 3; i >= 0; i--) {
		if(i != currentPage)
			CopyPage(i * 0x4000,currentPage * 0x4000);
	}
	
	SetupVMap(0);
	ShowPage(0);
	
}

void LoadFrame(char page, int handle, Handle bufferHandle)

{

	/* Display frame first reads the cel header from the curent file position
		and determines how much expanded cel data needs to be read and where
		it is to be positioned.  It then reads the data and sends the data
		a line at a time to the video driver for display */


	aCel theCel;
	uchar far * theBuffer;
	uint headerSize;
	uint count;

	headerSize = sizeof(aCel);
	theBuffer = *bufferHandle;

	count = ReadDos(handle,(uchar far *) &theCel,headerSize);

	if(count != headerSize) {
		DoAlert("Unable to Read Cel Header in DisplayFame");
		return;
	}

	// set the file position to the begining of the cel's actual data

	lseek(handle,theCel.dataOff,LSEEK_BEG);

	count = ReadDos(handle,theBuffer,(uint) theCel.compressSize);

	if(count != (uint) theCel.compressSize) {
		DoAlert("Error Reading Movie Frame");
		return;
	}

	// Now the buffer is filled with an uncompressed, cel of 
	// Display Block (cels.s) only sets up registers for the driver call

	xCel.pageOffset = (int) page * 0x4000;
	xCel.buffer = (char far*) theBuffer;
	xCel.dataOffset = (unsigned) theBuffer + (unsigned) theCel.controlSize;
	xCel.xOffset = theCel.xOff;
	xCel.yOffset = theCel.yOff;
	xCel.xDim = theCel.xDim;
	xCel.yDim = theCel.yDim;
	xCel.compressType = theCel.compressType;

++celNo;

	FillVideoPage(&xCel);

}


/*
 *
 *
 *
 *
 *  Server section
 *  ==============
 *
 *
 */
 

static long startTick;
static int currPage = 0;
static int lastPage = -1;


static void SetupMovieServer(void)
{
	int i;

	currPage = 0;
	lastPage = -1;
	startTick = 0;

	for (i = 0; i < 4; i++)  {
		pageStatus[i] = VACANT;
	}
	InstallServer(MovieServer, 1);
}


static void RemoveMovieServer(void)
{
	DisposeServer(MovieServer);
}


static void MovieServer(void)
{
	long tick;

	//  If nothing playing, ignore interrupt
	if (!movieOn)
		return;

	tick = RTickCount();
	if (tick >= startTick + frameRate) {		// time for display (10/second)

		//  If desired page available, display
		if (pageStatus[currPage] == FILLED)  {
			ShowPage(currPage * 0x4000);
			pageStatus[currPage] = IN_USE;

			//  Mark last displayed page as vacant (if any)
			if (lastPage != -1)
				pageStatus[lastPage] = VACANT;
			lastPage = currPage;

			//  Advance currPage to next one
			currPage++;
			if (currPage == 4)
				currPage = 0;

			startTick = tick;
		}
	}
}


/* LoadMoviePalette */

void LoadMoviePalette(int handle)
{

	long length;
	uint result;

	result = ReadDos(handle,(uchar far *) &length,4);
	if(result != 4) {
		DoAlert("Unable to Read Palette Length");
		return;
	}

	paletteHandle = ResLoad(RES_MEM,sizeof(RPalette));	// more than we need
	result = ReadDos(handle,*paletteHandle,(int) length);
	if(result != (uint) length) {
		DoAlert("Unable to Read Palette");
		return;
	}



}

void SetMoviePalette(void)

{
	picNotValid = FALSE;
	SubmitPalette((danPalette far *) *paletteHandle);
	ResUnLoad(RES_MEM,(uword) paletteHandle);
}

/* LoadCelCount */

unsigned LoadCelCount(int handle)
{

	unsigned celCount;
	uint result;

	result = ReadDos(handle,(uchar far *) &celCount,2);
	if(result != 2) {
		DoAlert("Unable to Read Cel Count");
		return 0;
	}

	return celCount;


}
