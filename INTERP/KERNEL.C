/* KERNEL.C
 * SCI kernel routines.
 */

#include	"kernel.h"

/* SCI-specific header files */

#include	"animate.h"
#include "audio.h"
#include	"cels.h"
#include "config.h"
#include	"debug.h"
#include	"dialog.h"
#include	"dongle.h"
#include	"errmsg.h"
#include	"event.h"
#include	"fardata.h"
#include	"fileio.h"
#include "getpath.h"
#include	"graph.h"
#include	"grtypes.h"
#include	"intrpt.h"
#include "kernel.h"
#include	"language.h"
#include	"list.h"
#include	"math.h"
#include	"menu.h"
#include	"mono.h"
#include	"mouse.h"
#include "movie.h"
#include	"memmgr.h"
#include "object.h"
#include	"picture.h"
#include	"pmachine.h"
#include	"resource.h"
#include	"restart.h"
#include "scale.h"
#include "scifgets.h"
#include	"script.h"
#include	"selector.h"
#include	"sound.h"
#include	"start.h"
#include "sync.h"
#include	"text.h"
#include "trig.h"
#include "view.h"
#include	"window.h"

/* ANSI standard header files */

#include	"ctype.h"
#include	"io.h"
#include	"stdio.h"
#include	"stdlib.h"
#include	"string.h"
#include	"types.h"

/* buffer for up to 256 bytes of NON-VOLATILE memory */
word		memorySegment[129];
/* global data related to the magnified cursor */

int loadLow = 0;
Hunkptr lowData;
uint		lowDataSize;
uint		magPicNum, pCursorNum;
int	magCursorFlag = 0;
Handle	cursorData;
Handle workPalette;
struct MagCursorStruct theMagCursorData;
struct pseudoCursor thePCursor;

int	noShowBits = 0;
uint	palStamp = 1;

#ifdef	DEBUG

#include	"sciglobs.h"
#define	STOP			0
#define	PLAY			1
#define	RECORD		2
#define	RECSTRLEN	120
	long	recordCnt;
	long	playbackCnt;
	uint	recordState = STOP;
	int	recordFd;
	int	recordRoom;
	long	recordOfs;
	int	recordX, recordY;
static strptr getstr(strptr str);
static uint getint(void);
static ulong getlong(void);
global void KRecord(argList);
global void KPlayBack(argList);

#else

global void KRecord(void);
global void KPlayBack(void);

#endif


/* Resource */

global void KAssertPalette(args)
argList;
{

Handle viewHandle;

	viewHandle = ResLoad(RES_VIEW,arg(1));
	AssertViewPalette(viewHandle,1);		// Force a palette update

}

global void KResCheck(args)
argList;
{
	int   count;

	switch (arg(1)) {
		case RES_SOUND:
			for (count = 2;count <= argCount;++count)
				{
				acc = (int) ResCheck(GetSoundResType((uint) arg(count)), (uint) arg(count));
				if (!acc)
					return;
	      	}
			break;
		case RES_AUDIO36:
		case RES_SYNC36:
			acc = (int) ResCheck36((ubyte) arg(1), (uint) arg(2), (uchar) arg(3),
					(uchar) arg(4), (uchar) arg(5), (uchar) arg(6));
			break;
		default:
			for (count = 2;count <= argCount;++count)
				{
				acc = (int) ResCheck((ubyte) arg(1), (uint) arg(count));
				if (!acc)
					return;
	      	}
			break;
	}
}


global void KLoad(args)
argList;
{
   int   count;
	ubyte	resType;

	switch (arg(1)) {
		case RES_SOUND:
			for (count = 2;count <= argCount;++count)
   		   {
				resType = GetSoundResType((uint) arg(count));
				if (ResCheck(resType, (uint) arg(count)))
		   		acc = Pseudo(ResLoad(resType, (uint) arg(count)));
				else
					acc = 0;
	      	}
			break;
		case RES_AUDIO36:
		case RES_SYNC36:
			break;
		default:
			for (count = 2;count <= argCount;++count)
   		   {
	   		acc = Pseudo(ResLoad((ubyte) arg(1), (uint) arg(count)));
	      	}
			break;
	}
}



global void KUnLoad(args)		/* resType, resId */
argList;
{
	switch (arg(1)) {
		case RES_MEM:
			UnloadBits((Handle) Native(arg(2)));
			break;
		case RES_SOUND:
			ResUnLoad(GetSoundResType((uint) arg(2)), (uint) arg(2));
			break;
		case RES_AUDIO36:
		case RES_SYNC36:
			break;
		default:
			ResUnLoad((ubyte) arg(1), (uint) arg(2));
			break;
	}
}



/* Lock or UnLock a resource */
/* pass resource type, resoure number and TRUE or FALSE to LOCK or UNLOCK */

global void KLock(args)
argList;
{
	ubyte	resType = (ubyte) arg(1);
	uword	resId = (uint) arg(2);
	bool	yes = (bool) arg(3);

	switch (arg(1)) {
		case RES_SOUND:
			resType = GetSoundResType(resId);
			break;
		case RES_AUDIO36:
		case RES_SYNC36:
			return;
		default:
			break;
	}
		
	if	(yes)
		ResLoad(resType, resId);
	ResLock(resType, resId, yes);
}



global void KSetCursor(args)
argList;
{
	Handle	cursor, magPic;
	RPoint	mousePoint;
	int 		bottom, right, xOff, yOff, mag, bigCelW, bigCelH ;
	int		smCelW, smCelH;	
 	ubyte		secondSkipCol;	
	Hunkptr	bigViewData;
	aCel  far *celPtr;

	switch (argCount) 
      {
		case 1:
         // show or hide cursor 
         // disable special effects cursor
         // restrict cursor to port
			switch (arg(1))
				{
					case -1:
					if(magCursorFlag)
						{
						ResUnLoad(RES_VIEW, magPicNum);
						ResUnLoad(RES_VIEW, pCursorNum);
						magCursorFlag = 0;
						}
					break;

					case -2:
						restrictRecPtr = 0;
					break;

					default:
				  		if (arg(1))
							RShowCursor();
					   else
							RHideCursor();
					break;
				}
			break;

		case 2:
         // move cursor
         /* The following code will restict the mouse cursor to the current port


         if (arg(1) < picWind->port.portRect.left) arg(1)= picWind->port.portRect.left;
         if (arg(1) > picWind->port.portRect.right) arg(1)= picWind->port.portRect.right - 1;
         if (arg(2) < picWind->port.portRect.top) arg(2)= picWind->port.portRect.top;
         if (arg(2) > picWind->port.portRect.bottom) arg(2)= picWind->port.portRect.bottom - 1;

         mousePoint.h = arg(1) + picWind->port.portRect.left;
         mousePoint.v = arg(2) + picWind->port.portRect.top;

         */
         mousePoint.h = arg(1);
         mousePoint.v = arg(2);

         SetMouse(&mousePoint);
			break;

		case 3:
         // set cursor
	      cursor = ResLoad(RES_VIEW, arg(1));
		   RSetCursor(cursor, arg(2), arg(3), -1, -1);
			break;

		case 4:
			/* restrict mouse to a given rectangle */
			restrict.top = arg(1);
			restrict.left = arg(2);
			restrict.bottom = arg(3);
			restrict.right = arg(4);
			restrictRecPtr = &restrict;
			break;

		case 10:
			/* special magnifier cursor */

			magCursorFlag = 1;

			mag	= arg(1);
			if((mag != 1) && (mag != 2) && (mag != 4) )
				Panic(E_MAGNIFY_FACTOR);
			xOff = arg(2);
			yOff = arg(3);
			right = arg(4);
			bottom = arg(5);
			secondSkipCol =   (ubyte) arg(10);

			PackHandles();
			magPicNum = arg(9);
			magPic = ResLoad(RES_VIEW, magPicNum);
			ResLock(RES_VIEW, magPicNum, TRUE);
			LockHandle(magPic);
			bigViewData = (Hunkptr) GetCelPointer( (Handle) magPic, 0, 0 );
			celPtr =  (aCel far *) bigViewData;

			theMagCursorData.picture = *magPic + (uint) celPtr->dataOff;

			bigCelW = celPtr->xDim;
			bigCelH = celPtr->yDim;
	  		smCelW  = right - xOff;
			smCelH  = bottom - yOff;

			if(celPtr->compressType) {
				Panic(E_MAGNIFY_CEL);
			}

			/* Make sure dimensions are in sync */

			if( ((bigCelW / smCelW != mag) || (bigCelW % smCelW))	  ||
				 ((bigCelH / smCelH != mag) || (bigCelH % smCelH)) 
			  )
				Panic(E_MAGNIFY_SIZE);
			
		 		/* load the magnifier cursor in hunk */

			pCursorNum = arg(6);
			cursor = ResLoad(RES_VIEW,	pCursorNum);
			ResLock(RES_VIEW, pCursorNum, TRUE);
			LockHandle(cursor);

			bigViewData = (Hunkptr) GetCelPointer( (Handle) cursor,0,0);
			celPtr =  (aCel far *) bigViewData;

			if(celPtr->compressType) {
				Panic(E_MAGNIFY_CEL);
			}

			thePCursor.xDim = celPtr->xDim;
			thePCursor.yDim = celPtr->yDim;

			thePCursor.xHot = (celPtr->xDim ) / 2 -  (int) ((signed char ) (celPtr->xOff));
			thePCursor.yHot = celPtr->yDim - 1 - celPtr->yOff;

			thePCursor.skipColor = celPtr->skipColor;
			if(right > 319) right = 319;
			if(bottom > 189) bottom = 189;
			mousePoint.h = xOff + 10  ;
         mousePoint.v = yOff + 10  ;
         SetMouse(&mousePoint);

			/* Fill in data structure for the special effects cursor */

			theMagCursorData.curTop = yOff;
			theMagCursorData.curLeft = xOff;
			theMagCursorData.curBottom = bottom;
			theMagCursorData.curRight = right;
			theMagCursorData.cursor = *cursor + (uint) celPtr->dataOff;
			theMagCursorData.picXDim = bigCelW;
			theMagCursorData.replaceColor = secondSkipCol;
			theMagCursorData.magLevel = mag ;
			break;

      }
}


void
KDrawPic(argList)	/* pic [showStyle] */
{
	RGrafPort *oldPort;
	Handle BTargetPalette;

	/* defaults for optional arguments */
	word	clear = TRUE;

	++palStamp;
	if(!palStamp)
		palStamp++;

	RGetPort(&oldPort);
	RSetPort((RGrafPort *) picWind);


	/* get optional show style if present */
	if (argCount >= 2){
		showStyle = arg(2);
		/* get optional overlay boolean */
		if (argCount >= 3)
			clear = arg(3);
	}
	/* get rid of previous target palette */
	if (!gameRestarted)
		{
		paletteRes = arg(1);
		if (palVaryOn)
			{
			BTargetPalette = ResLoad(RES_PAL, paletteRes);
			if(BTargetPalette) {
				CopyNew2OldPalette((RPalette far *) *targetPalette,(danPalette far *) *BTargetPalette);
				PaletteUpdate(NOSETCLUT,0);		// updates sysPalette
				ResUnLoad(RES_PAL, paletteRes);
			}
			else
				RAlert(E_NO_NIGHT_PAL);
		}
	}
	if (!RFrontWindow(picWind)){
		RBeginUpdate(picWind);
											 /* showStyle contains mirroring bits */
		DrawPic(ResLoad(RES_PIC, arg(1)), clear, showStyle);

		REndUpdate(picWind);
	} else {
		DrawPic(ResLoad(RES_PIC, arg(1)), clear, showStyle);
		picNotValid = 1;
	}
	RSetPort(oldPort);

}



global void KPicNotValid(args)		/* return (or set) picture validity */
argList;
{
	acc = noShowBits;
	if (argCount)
		noShowBits = arg(1); 
}



global void KShow(args)		/* virtual bitmap to show */
argList;
{
	RGrafPort *oldPort;

	RGetPort(&oldPort);
	RSetPort((RGrafPort *) picWind);
	picNotValid = 2;
	showMap = (uword) arg(1);
	RSetPort(oldPort);
}



global void KTextSize(args)		/* size rectangle for this text */
argList;
{
	word def = 0;

	if (argCount >= 4)
		def = arg(4);

	RTextSize( (RRect *) Native(arg(1)), Native(arg(2)), arg(3), def);
}



global void KAnimate(args)		/* animate this cast list */
argList;
{
	 RRect r, sumRect;
	 static RRect oldRect;
	 static Handle unders = 0;
	 int	theMouseX, theMouseY;

	/* If we are currently in a special effects 
      cursor mode process accordingly */

	if(magCursorFlag)	 /* Perform magnified cursor operation  */
		{
			theMouseX = mouseX;
			theMouseY = mouseY;

			if(unders)
				RestoreBits(unders);
			
			/* Keep our psuedo cursor within specified port */

		  	if(theMouseX < theMagCursorData.curLeft) theMouseX = theMagCursorData.curLeft;
			if(theMouseX > theMagCursorData.curRight) theMouseX = theMagCursorData.curRight;
			if(theMouseY < theMagCursorData.curTop) theMouseY = theMagCursorData.curTop;
			if(theMouseY > theMagCursorData.curBottom) theMouseY = theMagCursorData.curBottom;

			/* Construct rect for saving underbits*/

			r.left = theMouseX - thePCursor.xHot ;
			r.top = theMouseY - thePCursor.yHot - 10 ;
			r.right = r.left + thePCursor.xDim ;
			r.bottom = r.top + thePCursor.yDim ;

			/* Save underbits*/
			unders = SaveBits(&r, VMAP); 
			/*Actually draw our special effects cursor into the VMAP*/
			DrawMagCursor(&theMagCursorData, &thePCursor, theMouseX, theMouseY);
			/*Construct and show rect that will result in a ShowBits*/
			RUnionRect(&r,&oldRect, &sumRect);
			ShowBits(&sumRect, VMAP);
			RCopyRect(&r, &oldRect);

		}
	else
	   {
      if(unders) 
			{
			/* Discard any left over underbits*/
			UnloadBits(unders);
			unders = 0;
			}
      }

	if (argCount == 2)
      {
		PaletteCheck(); 
		Animate((List *) Native(arg(1)), arg(2));
      }
	else
		Animate((memptr) 0, 0);
}



global void KGetAngle(args)
argList;
{
	RPoint sp, dp;

	sp.h = arg(1);
	sp.v = arg(2);
	dp.h = arg(3);
	dp.v = arg(4);
	acc = RPtToAngle(&sp, &dp);
}



global void KGetDistance(args)
argList;
/*
	Compute distance between two points taking perspective into account
	perspective is optional fifth arg, default is zero
	perspective = departure from vertical along Y axis, in degrees
*/
{
	long dx, dy;

	dx = (long) abs(arg(3)-arg(1));
	dy = (long) abs(arg(4)-arg(2));
	
	if (argCount>4) 
		dy = CosDiv(arg(5), dy);
		
	acc = sqrt((dx * dx) + (dy * dy));
}

global void KSinMult(args)
argList;
	{
	acc = SinMult(arg(1),arg(2));
	}

global void KCosMult(args)
argList;
	{
	acc = CosMult(arg(1),arg(2));
	}

global void KSinDiv(args)
argList;
	{
	acc = SinDiv(arg(1),arg(2));
	}

global void KCosDiv(args)
argList;
	{
	acc = CosDiv(arg(1),arg(2));
	}

global void KATan(args)
argList;
	{
	acc = ATan(arg(1),arg(2),arg(3),arg(4));
	}


global void KDisplay(args)	/* textNum [at:] [color:] [back:] [noshow:] */
argList;
{
	word		*lastArg;
	word		mess;

	/* default width to no wrap and only width of passed text */
	word		width;
	bool		saveIt;
	word		colorIt;
	word		mode;
	bool		showIt;
	strptr	text;
	char		buffer[1000];
	RRect		r;
	RRect far	*sRect;
	Handle	bits;
 	uword		arg1;
	RGrafPort	savePort;
	RPoint		newPenLoc;

	width = colorIt = -1;
	mode = TEJUSTLEFT;
	saveIt = FALSE;
	showIt = TRUE;

	lastArg = (args + arg(0));

	/* text string is first parameter */
	/* uses technique documented in Format to handle far text */
	arg1 = arg(1);
	if (arg1 < 1000) {
		text = GetFarText(arg1, arg(2), buffer);
		++args;
		}
	else
		text = Native(arg1);

	++args;

	/* Save the contents of the current grafport.
	 */
	savePort = *rThePort;

	/* set all defaults */
	RPenMode(SRCOR);
	PenColor(0);
	RTextFace(0);

	/* check for and accomodate optional parameters */
	while (args <= lastArg){
		mess = *args;
		++args;
		switch (mess){
			case p_at:
				RMoveTo(*args, *(args + 1));
				args += 2;
				break;
			case p_mode:
				mode = *args++;
				break;
			case p_color:
				PenColor(*args++);
				break;
			case p_back:
				/* set transfer mode to copy so it shows */
				colorIt = *args++;
				break;
			case p_style:
				RTextFace(*args++);
				break;
			case p_font:
				RSetFont(*args++);
				break; 
			case p_width:
				width = *args++;
				break;
			case p_save:
				saveIt = TRUE;
				break;
			case p_noshow:
				showIt = 0;
				break;
			case p_restore:
				/* get the rectangle at the start of the save area */
				if (FindResEntry(RES_MEM, *args)) {
					bits = (Handle) Native(*args++);
					sRect = (RRect far *) *bits;
					/* this rectangle is in Global coords */
					r.top = sRect->top;
					r.left = sRect->left;
					r.bottom = sRect->bottom;
					r.right = sRect->right;

					/* restore the virtual maps */
					RestoreBits(bits);

					/* ReDraw this rectangle in Global coords */
					RGlobalToLocal((RPoint *) &r.top);
					RGlobalToLocal((RPoint *) &r.bottom);
					ReAnimate(&r);
				}
 
				/* no further arguments accepted */
				return;

		}
	}
	/* get size of text rectangle */
	RTextSize(&r, text, -1, width);

	/* move the rectangle to current pen position */
	MoveRect(&r, rThePort->pnLoc.h, rThePort->pnLoc.v);

	/* keep rectangle within the screen - Pablo Ghenis 10-1-90 */
	RMove(	r.right > 320  ? 320 - r.right : 0,
			r.bottom > 200 ? 200 - r.bottom: 0);
	MoveRect(&r, rThePort->pnLoc.h, rThePort->pnLoc.v); /*reposition*/

	if (saveIt)
		acc = Pseudo(SaveBits(&r, VMAP));

	/* opaque background */
	if (colorIt != -1)
		RFillRect(&r, VMAP, colorIt);

	/* now draw the text and show the rectangle it is in */
	RTextBox(text, FALSE, &r, mode, -1);

	/* don't show if the picture is not valid */
	if ( (!picNotValid) && (showIt) )
		ShowBits(&r, VMAP);

	/* Restore contents of the current port.
	 */
	newPenLoc = rThePort->pnLoc;
	*rThePort = savePort;
	rThePort->pnLoc = newPenLoc;
} 



global void KHaveMouse()	/* return TRUE if we have a mouse available */
{
	acc = haveMouse;	/* public in eventasm */
}



global void KGetEvent(args)				/* fill in an Event Object */
argList;
{
	REventRecord evt;
	uint			type;

#ifdef DEBUG
	char	saveStr[RECSTRLEN];
#endif

	type = arg(1);
	if (type & leaveEvt)
		acc = REventAvail(type, &evt);
	else
		acc = RGetNextEvent(type, &evt);

#ifdef DEBUG
	if(recordState != STOP)
		recordCnt++;
	switch(recordState) {
		case RECORD:
			if(recordRoom != globalVar[g_curRoomNum]) {
				recordRoom = globalVar[g_curRoomNum];
				sprintf(saveStr, ";Current room = %d\r\n", recordRoom);
				write(recordFd, saveStr, strlen(saveStr));
			}
			// Save Offset to clear out last event
			recordOfs = lseek(recordFd, 0L, LSEEK_CUR);
			if((evt.type != nullEvt) || (recordX != mouseX) || (recordY != mouseY)) {
				recordX = mouseX;
				recordY = mouseY;
				sprintf(saveStr, "%U\r\n%u\r\n%u\r\n%u\r\n%u\r\n%u\r\n\r\n",
					recordCnt,
					evt.type,
					evt.message,
					evt.modifiers,
					evt.where.v,
					evt.where.h
				);
				write(recordFd, saveStr, strlen(saveStr));
			}
			break;
		case PLAY:
			if(recordCnt == playbackCnt) {
				evt.type = getint();
				evt.message = getint();
				evt.modifiers = getint();
				evt.when = RTickCount();
				evt.where.v = getint();
				evt.where.h = getint();
				getstr(saveStr);			// read blank line
				playbackCnt = getlong();
				MoveCursor(mouseX = evt.where.h, mouseY = evt.where.v);
			}
			break;
	}
#endif

	EventToObj(&evt, (Obj *) Native(arg(2)));
}

#ifdef DEBUG
static strptr getstr(strptr str)
{
	do {
		if(!sci_fgets(str, RECSTRLEN, recordFd)) {
			recordState = STOP;
			return(NULL);
		}
	} while(*str == ';');
	return(str);
}

static uint getint(void)
{
	char	str[RECSTRLEN];

	if(!getstr(str)) return(0);
	return(atoi(str));
}

static ulong getlong(void)
{
	char	saveStr[RECSTRLEN];
	char	*str = saveStr;
	ulong	n;

	if(!getstr(str)) return(0L);

	/* Scan off spaces
	 */
	for ( ; isspace(*str); str++)
		;

	/* Initialize.
	 */
	n = 0;

	while (isdigit(*str))
		n = 10*n + *str++ - '0';

	return(n);
}

// (Record flag <filename>)
global void KRecord(args)
argList;
{
	char file[RECSTRLEN];

	if(argCount < 1) return;
	if(!arg(1)) {
		if(recordState == RECORD) {
			recordState = STOP;

			// Clear last event (record toggle)
			lseek(recordFd, recordOfs, LSEEK_BEG);
			write(recordFd, ";", sizeof(char));	//recordCnt
			getstr(file);
			write(recordFd, ";", sizeof(char));	//type
			getstr(file);
			write(recordFd, ";", sizeof(char));	//message
			getstr(file);
			write(recordFd, ";", sizeof(char));	//modifiers
			getstr(file);
			write(recordFd, ";", sizeof(char));	//when
			getstr(file);
			write(recordFd, ";", sizeof(char));	//where.v
			getstr(file);
			write(recordFd, ";", sizeof(char));	//where.h

			close(recordFd);
		}
	} else {
		if(recordState == STOP) {
			strcpy(file, (argCount >= 2) ? (char *)arg(2) : "sci_evt.mac");
			recordState = RECORD;
			recordCnt = 0L;
			recordFd = creat(file, 0);
			recordRoom = globalVar[g_curRoomNum];
			recordX = mouseX;
			recordY = mouseY;
			sprintf(file, ";Current room = %d\r\n", recordRoom);
			write(recordFd, file, strlen(file));
		}
	}
	acc = (recordState == RECORD);
}

// (PlayBack flag <filename>)
global void KPlayBack(args)
argList;
{
	char file[RECSTRLEN];

	if(argCount < 1) return;
	if(!arg(1)) {
		if(recordState == PLAY) {
			recordState = STOP;
			close(recordFd);
		}
	} else {
		if(recordState == STOP) {
			strcpy(file, (argCount >= 2) ? (char *)arg(2) : "sci_evt.mac");
			recordState = PLAY;
			recordFd = open(file, 0);
			recordCnt = 0L;
			playbackCnt = getlong();
		}
	}
	acc = (recordState == PLAY);
}
#else
global void KRecord(void)
{
}

global void KPlayBack(void)
{
}
#endif


global void KGetTime(args)	/* return low word of system ticks OR packed real time */ 
argList;
{
	if (argCount == 1)
		acc = SysTime(arg(1));
	else
		acc = (uword) RTickCount();
}



global void KNumLoops(args)
argList;
{
	Obj	*him = (Obj *) Native(arg(1));

	acc = GetNumLoops(ResLoad(RES_VIEW, GetProperty(him, s_view)));
}



global void KNumCels(args)
argList;
{
	Obj	*him = (Obj *) Native(arg(1));

	acc = GetNumCels(ResLoad(RES_VIEW, GetProperty(him, s_view)),
				GetProperty(him, s_loop));

}


global void KSetNowSeen(args)
argList;
{
	Obj	*him = (Obj *) Native(arg(1));

	GetCelRect(ResLoad(RES_VIEW, GetProperty(him, s_view)), 
			GetProperty(him, s_loop),
			GetProperty(him, s_cel),
			GetProperty(him, s_x),
			GetProperty(him, s_y),
			GetProperty(him, s_z),
			(RRect *) GetPropAddr(him, s_nsTop));
}



global void KIsItSkip(args)
argList;
{
	Handle viewHandle;

	viewHandle = ResLoad(RES_VIEW,arg(1));

	if(RIsItSkip(viewHandle,arg(2),arg(3),arg(4),arg(5)))	 // View,loop,cel,vOffset,hOffset
		acc = TRUE;
	else
		acc = FALSE;

}


global void KCelWide(args)
argList;
{
	acc = GetCelWide(ResLoad(RES_VIEW, arg(1)), arg(2), arg(3));
}



global void KCelHigh(args)
argList;
{
	acc = GetCelHigh( ResLoad(RES_VIEW, arg(1)), arg(2), arg(3));
}


/* determine and return legality of actors position */
/* This code NOW checks controls AND base rect intersection */
global void KCantBeHere(args)
argList;
{
	Obj	*him = (Obj *) Native(arg(1));
	List *cast = (List *) Native(arg(2));
	ObjID node;
	Obj	*me;
	RRect r, *chkR;
	RGrafPort *oldPort;


	RGetPort(&oldPort);
	RSetPort((RGrafPort *) picWind);
	RCopyRect((RRect *) IndexedPropAddr(him, actBR), &r);

	/* (s_illegalBits) are the bits that the object cannot be on.
	 * Anding this with the bits the object is on tells us which
	 * bits the object is on but shouldn't be on.  If this is zero,
	 * the position is valid.
	 */
	acc = 0;
	if (! (acc = (OnControl(CMAP, &r) & IndexedProp(him, actIllegalBits)))){

		/* controls were legal, do I care about other actors? */
		/* if I am hidden or ignoring actors my position is legal 8 */
 		if (! (IndexedProp(him, actSignal) & (ignrAct | HIDDEN))){

			/* default to no hits */
			acc = 0;
			/* now the last thing we care about is our rectangles */
			for (node = FirstNode(cast) ;  node ;  node = NextNode(node) ) {
				me = (Obj *) Native(((KNode *)Native(node))->nVal);
		
				/* Can't hit myself */
				if (him == me)
					continue;

				/* can't hit if I'm as below */
				if ((IndexedProp(me, actSignal) & (NOUPDATE | ignrAct | HIDDEN)))
					continue;

				/* check method from actor */
		
				/* if our rectangles intersect we are done */
				chkR = (RRect *) IndexedPropAddr(me, actBR);
				if (r.left >= chkR->right
				|| r.right <= chkR->left
				|| r.top >= chkR->bottom
				|| r.bottom <= chkR->top){
					continue;
				} else {
					acc = Pseudo(me);
					break;
				}
			}
		}
	}

	RSetPort(oldPort);
}



global void KOnControl(args)	/* return V|P|C bits encompassed by rectangle */
argList;
{			
	RRect r;
	RGrafPort *oldPort;

	RGetPort(&oldPort);
	RSetPort((RGrafPort *) picWind);
	
	/* first argument is bitmap to examine */

	/* next two arguments are X/Y of upper/left of rectangle */
	r.left = arg(2);
	r.top = arg(3);

	/*  optional fourth and fifth arguments are X/Y of bottom/right */
	if (argCount == 5){
		r.right = arg(4);
		r.bottom = arg(5);

	} else {

	/* make rectangle enclose one point */
		r.right = r.left + 1;
		r.bottom = r.top + 1;
	}

/* call the real function with this rectangle */
	acc = OnControl(arg(1), &r);
	RSetPort(oldPort);
}


global void KGetPort()
{
	RGrafPort *oldPort;

	RGetPort(&oldPort);
	acc = Pseudo(oldPort);
}



global void KSetPort(args)
argList;
/* Calls to KSetPort with an argument count of 6 or more are assumed to be calls
** to SetPortRect.
*/
{
	if (argCount >= 6){
		picWind->port.portRect.top = arg(1);
		picWind->port.portRect.left = arg(2);
		picWind->port.portRect.bottom = arg(3);
		picWind->port.portRect.right = arg(4);
		picWind->port.origin.v	= arg(5);
		picWind->port.origin.h	= arg(6);
		if (argCount >= 7){
			InitPicture();
		}

	} else 
		if (arg(1))
			if ((arg(1)) == -1)
				RSetPort(menuPort);
			else 
				RSetPort((RGrafPort *) Native(arg(1)));
		else
			RSetPort((RGrafPort *) RGetWmgrPort());
}


global void KNewWindow(args)
argList;
{
	/* ARGS: top, left, bottom, right, title, type, priority, color, back */
	/* open the window with NO draw (vis false), then set port values */
	RRect r, uR, *underR;

	
	r.top 	= 	arg(1);
	r.left 	= 	arg(2);
	r.bottom = 	arg(3);
	r.right	= 	arg(4);

	uR.top 		= 	arg(5);
	uR.left 		= 	arg(6);
	uR.bottom 	= 	arg(7);
	uR.right		= 	arg(8);

	if (uR.top | uR.left | uR.right | uR.bottom)
		underR = &uR;
	else
		underR = NULL;

	acc = Pseudo( RNewWindow(&r, underR, Native(arg(9)), arg(10), arg(11), FALSE));


	/* set port characteristics based on remainder of args */
	rThePort->fgColor = arg(12);
	rThePort->bkColor = arg(13);
	RDrawWindow((RWindow *) Native(acc));
}

global void KDisposeWindow(args)	/* close the window */
argList;
{
	bool eraseOnly;

	eraseOnly = (argCount == 2 && arg(2));
	RDisposeWindow( (RWindow *) Native(arg(1)), eraseOnly);
		
}



void		
KDrawControl(args)	/* draw a control object in current port */
argList;
{
   word *nRect;
	nRect = DrawControl( (Obj *) Native(arg(1)));
   acc = (word)nRect;
}



void		
KHiliteControl(args)	/* do proper hilighting of control */
argList;
{
	RHiliteControl( (Obj *) Native(arg(1)));
}



global void KEditControl(args)	/* call the kernel to edit an EditText item */
argList;
{
	acc = EditControl( (Obj *) Native(arg(1)), (Obj *) Native(arg(2)));
}



global void KGlobalToLocal(args)	/* adjust the event record local coords */
argList;
{
	RGlobalToLocal((RPoint *) IndexedPropAddr( (Obj *) Native(arg(1)), evY));
}



global void KLocalToGlobal(args)	/* adjust the event record global coords */
argList;
{
	RLocalToGlobal((RPoint *) IndexedPropAddr( (Obj *) Native(arg(1)), evY));
}

global void KDrawCel(args)	/* draw cel in current port at coords and pri */
argList;
{
	RRect		r;
	Handle	view;
	uint		loop, cel;
   int      scaleDraw = 0;

   word newXDim, newYDim, oldXDim, oldYDim, oldXOff, oldYOff;
   word  newXOff, newYOff;
	word theScaleX, theScaleY;


	theViewNum = arg(1);
	view = ResLoad(RES_VIEW, theViewNum);
	loop = arg(2);
	theLoopNum = loop;
	cel = arg(3);
	theCelNum = cel;

   if(argCount == 8 && arg(7) != NULL)
      scaleDraw = 1;


	r.left = arg(4);
	r.top  = arg(5);


   if(scaleDraw)
      {
      theScaleX = arg(7);
      theScaleY = arg(8);

      oldXDim = GetCelWide(view,loop,cel);
		oldYDim = GetCelHigh(view,loop,cel);

      oldXOff = GetCelXOff(view,loop,cel);  
      oldYOff = GetCelYOff(view,loop,cel);

      vm_size(oldXDim,oldYDim, oldXOff, oldYOff,theScaleX, theScaleY,&newXDim, &newYDim, &newXOff ,&newYOff);
	 	SetNewDim(view,loop, cel, newXDim, newYDim, newXOff, newYOff); 

      r.right = r.left + newXDim;
      r.bottom = r.top + newYDim;
		ScaleDrawCel((Handle) view,loop,cel, &r, arg(6),oldXDim);
	  	SetNewDim(view,loop, cel, oldXDim, oldYDim,oldXOff ,oldYOff ); 

      }
   else
      {
	   r.right = r.left + GetCelWide(view, loop, cel);
	   r.bottom = r.top + GetCelHigh(view, loop, cel);
	   DrawCel((Handle) view, loop, cel, &r, arg(6));
	   /* show results if picture IS valid */
	   
      }

      if (!noShowBits)
		   ShowBits(&r, VMAP);
}

global void KAddToPic(args)		/* add this view to current picture */
argList;
{
	AddToPic((List *) Native(arg(1)));
	picNotValid = 2;
}


global void KRandom(args)	/* low, high */
argList;
{
	uint	low, high, range;
	ulong	tmp;

	if (argCount == 2) {
		low = arg(1);
		high = arg(2);
		range = high - low + 1;

	   tmp = (LCGRandom() & 0x00ffff00) >> 8;
      tmp *= range;
      tmp >>= 16;
      tmp += low;

	   acc = (uint) tmp;
  	} else if (argCount == 1) {
      /* Set seed to argument */
      lcgSeed = *(long *)Native(arg(1));
	   acc = FALSE;
   } else if (argCount == 3) {
      *(long *)Native(arg(3)) = lcgSeed;
      acc = FALSE;
   } else {
         /* assume argCount == 0 */
         /* Return seed value */
	      acc = (uint)lcgSeed;
   }
}


global void KNewList()		/* create a new list and return its address */
{
	List	*list;

	list = NeedPtr(sizeof(List));
	InitList(list);
	acc = Pseudo(list);
}



global void KDisposeList(args)	/* dispose of allocated memory associated with kList */
argList;
{
	List	*list;
	ObjID kNode;

	list = (List *) Native(arg(1));

	while (!EmptyList(list)) {
		kNode = FirstNode(list);
		DeleteNode(list, kNode);
		DisposePtr(Native(kNode));
		}

	DisposePtr(list);
}



global void KNewNode(args)
argList;
{
	KNode	*node;

	node = (KNode *) NeedPtr(sizeof(KNode));
	node->nVal = arg(1);
	acc = Pseudo(node);
}



global void KFirstNode(args)
argList;
{
	acc = (arg(1) != NULL)? FirstNode(Native(arg(1))) : 0;
}



global void KLastNode(args)
argList;
{
	acc = (arg(1) != NULL)? LastNode(Native(arg(1))) : 0;
}



global void KEmptyList(args)
argList;
{
	acc = (arg(1) == NULL) || EmptyList(Native(arg(1)));
}



global void KNextNode(args)
argList;
{
	acc = NextNode(arg(1));
}



global void KPrevNode(args)
argList;
{
	acc = PrevNode(arg(1));
}



global void KNodeValue(args)
argList;
{
	acc = ((KNode *) Native(arg(1)))->nVal;
}



global void KAddAfter(args)
argList;
{
	acc = AddAfter((List *) Native(arg(1)), arg(2), arg(3), arg(4));
}



global void KAddToFront(args)
argList;
{
	acc = AddToFront((List *)Native(arg(1)), arg(2), arg(3));
}



global void KAddToEnd(args)
argList;
{
	acc = AddToEnd((List *)Native(arg(1)), arg(2), arg(3));
}



global void KFindKey(args)
argList;
{
	acc = FindKey((List *)Native(arg(1)), arg(2));
}



global void KDeleteKey(args)
argList;
{
	ObjID theNode;

	theNode = DeleteKey((List *)Native(arg(1)), arg(2));
	if (theNode)
		DisposePtr(Native(theNode));
	acc = (theNode != NULL);
}



global void KAbs(args)
argList;
{
	acc = abs(arg(1));
}



global void KSqrt(args)
argList;
{
	acc = sqrt((ulong) abs(arg(1)));
}



global void KReadNumber(args)	/* read a number from the input line */
argList;
{
	acc = atoi(Native(arg(1)));
}



global void KClone(args)
word	*args;
{
	Obj	*theClone;
	int	numArgs;

	/* Get a clone of the object.
	 */
	theClone = Clone((Obj *) Native(arg(1)));

	/* Set any properties.
	 */
	for (
			numArgs = argCount - 1, args += 2 ;
			numArgs > 0 ;
			numArgs -= 2, args += 2
	    )
		    SetProperty(theClone, arg(0), arg(1));

	acc = Pseudo(theClone);
}



global void KDisposeClone(args)
argList;
{
	DisposeClone((Obj *) Native(arg(1)));
}



global void KMemoryInfo(args)
word	*args;
{
	ulong	size;

	switch (arg(1)) {
		case LARGESTPTR:
			acc = LargestPtr();
			break;
		case FREEHEAP:
			acc = FreeHeap();
			break;
		case LARGESTHANDLE:
			size = LargestHandle();
			acc = (size > (ulong) 0xffff)? 0xffff : (uint) size;
			break;
		case FREEHUNK:
			acc = FreeHunk();
			break;
		case TOTALHUNK:
			acc = hunkAvail;
			break;
		}
}



global void KScriptID(args)
argList;
{
	acc = Pseudo(GetDispatchAddr(arg(1), (argCount == 1)? 0 : arg(2)));
}



global void KWait(args)		/* wait the specified number of ticks before returning */
argList;
{
	static	ulong	lastTick, ticks;

	ticks = (ulong) arg(1);

	if (ticks)
		while (ticks + lastTick > sysTicks)
			;

	acc = (word) (sysTicks - lastTick);
	lastTick = sysTicks;
   /* This test prevents beta test versions 
      from being pirated and other users from using
      this interpreter with any other game then the
      one it was built for */
   /* See start.s and pmachine.s for other parts of this test */
   if (noDongle)
      {
      if (!(--noDongle))
         {
		   PError(thisIP,pmsp,E_NO_DONGLE,gameCode,0);
         }
      }
}

global void KFormat(args)
argList;
{
	strptr	text, format;
	word		i, n;
	uint		theArg;
	char		c, *strArg, theStr[20], buffer[2000], temp[500];
	

	text = Native(arg(1));
	format = Native(arg(2));

	/* The following code is quite a kludge.  It relies on the fact that a
	 * valid Script address will be greater than 1000 to let us distinguish
	 * between a near string address and the module number of a far string.
	 * If we're dealing with a far string, we copy it into 'buffer' to make
	 * it near for formatting purposes.
	 */
	if (Pseudo(format) < 1000) {
		format = GetFarText(arg(2), arg(3), buffer);
		n = 4;
		}
	else
		n = 3;

	/* Format message. */
	for ( ; (c = *format++) != '\0' ; ) {
		if (c != '%')
			*text++ = c;
		else {
			/* Build a string to pass to sprintf.
			 */
			theStr[0] = '%';
			for (i = 1 ; i < sizeof(theStr) ; i++) {
				c = *format++;
				theStr[i] = c;
				if (isalpha(c)) {
					theStr[i + 1] = 0;
					theArg = arg(n++);
					if (c == 's') {
						if (theArg < 1000)
							strArg = GetFarText(theArg, arg(n++), temp);
						else
							strArg = Native(theArg);
						text += sprintf(text, theStr, strArg);
						}
					else
						text += sprintf(text, theStr, theArg);
					break;
					}
				}
			}
		}

	*text = 0;
	acc = arg(1);
}



global void KIsObject(args)
argList;
{
	acc = IsObject((Obj *) Native(arg(1)));
}



global void KRespondsTo(args)
argList;
{
	acc = RespondsTo((Obj *) Native(arg(1)), arg(2));
}



global void KStrCmp(args)
argList;
{
	if (argCount == 2)
		acc = strcmp(Native(arg(1)), Native(arg(2)));
	else
		acc = strncmp(Native(arg(1)), Native(arg(2)), arg(3));
}



global void KStrLen(args)
argList;
{
	acc = strlen(Native(arg(1)));
}



global void KStrCpy(args)
argList;
{
	strptr src, dst;
	word i;

	acc = arg(1);
	if (argCount == 2)
		strcpy(Native(arg(1)), Native(arg(2)));
	else {
		/* a positive length does standard string op */
		if (arg(3) > 0)
			strncpy(Native(arg(1)), Native(arg(2)), arg(3));
		else {
		/* a negative length does mem copy without NULL terminator */
			src = Native(arg(2));
			dst = Native(arg(1));
			i = abs(arg(3));
			while(i--)
				*dst++ = *src++;
		}
	}
}



global void KStrEnd(args)
argList;
{
	strptr	str;

	for (str = Native(arg(1)) ; *str ; ++str)
		;

	acc = Pseudo(str);
}



global void KStrCat(args)
argList;
{
	acc = arg(1);
	strcat(Native(acc), Native(arg(2)));
}



void KDummy()
             
{
}



global void KGetCWD(args)
argList;
{
	acc = arg(1);
	getcwd(Native(acc));
}


global void KCoordPri(args)
argList;
{
	if (argCount >= 2 && arg(1) == PTopOfBand)
		acc = PriCoord(arg(2));
	else
		acc = CoordPri(arg(1));
}




global void KShakeScreen(args)
argList;
{
	/* shake screen the arg(1) times in optional direction */
	word	dir = 1;

	if (argCount == 2)
		dir = arg(2);

	ShakeScreen(arg(1), dir);
}
global void KStrAt(args)
argList;
/* Return the nth [arg(2)] byte of a string [arg(1)].  If a third argument
 * is present, set the byte to that value.
 */
{
	memptr	sp;

	sp = (memptr) Native(arg(1)) + (int) arg(2);
	acc = *sp;
	if (argCount == 3)
		*sp = (ubyte) arg(3);
}


/*------------------------------------------------------------------*/
/* shift the screen area defined by rect in the specified direction */
/* arg(1) = rect top																  */
/* arg(2) = rect left														     */
/* arg(3) = rect bottom											 				  */
/* arg(4) = rect right												  			  */
/* arg(5) = dir																	  */
/*------------------------------------------------------------------*/
global void KShiftScreen(args)
argList;
{
	ShiftScreen(arg(1),arg(2),arg(3),arg(4),arg(5));
}


global void KFlushResources(args)
argList;
/* If we are tracking resource usage, set the room number of the room
 * we are about to enter, then flush all unlocked resources.
 */
{
	newRoomNum = arg(1);

	if (trackResUse) {
		while (!PurgeLast())
			;
		}
}


global void KMemorySegment(args)
argList;
/* Allow up to 256 bytes of game data to be preserved when a restart/restore
 * is performed. The first argument is a function code that determines if
 * data is being SAVED FROM the game or being RESTORED TO it.
 *		SAVE FROM:
 *			Argument 2 - pointer to data area to be saved.
 *			Argument 3 - size in bytes of data to be saved; 0 size indicates a
 *							 string is to be saved.	If the size of the data to be
 *							 saved exceeds 256 bytes, then only the first 256 bytes
 *							 will be saved.
 *		RESTORE TO:			             
 *			Argument 2 - pointer to data area to receive saved data.
 *			Argument 3 - *** NOT required because the RESTORE TO function uses
 *							 the size parameter from the last SAVE FROM function to
 *							 determine the size of the data area to be restored.
 */
{

	strptr	s, d;
	int		size, i;

	acc = arg(2);
	if (arg(1) == SAVE_FROM)
		{
			s = (strptr) Native(acc);
			d = (strptr) &memorySegment[1];
			if ((size = arg(3)) == STRING)
				size = (strlen(s) + 1);
			if (size > 256)
				size = 256;
			memorySegment[0] = size;
		}
	else
	 	{
			s = (strptr) &memorySegment[1];
			d = (strptr) Native(acc);
			size = memorySegment[0];
		}
	for (i = 0; i < size; i++)
		*d++ = *s++;
}


/* if three args only are passed, then this function just returns TRUE or FALSE
** indicating if the x,y were in the polygon	(arg(3)), else it returns the full  
**	path (either optimized or unoptimized) around the list of polygons passed.
*/

global void KAvoidPath(args)
argList;
{	
	register 	Obj	*him;	
	register 	ObjID it;
	List 			*theList;
	AvdPoint 	A,B,*P;
	polygon		*polylist;
	int 			size, i,opt;

	A.x 		= 	arg(1);
	A.y 		= 	arg(2);
	acc 		= 	0;
	if (argCount >= 4){ /* wants full path */
		B.x 		= 	arg(3);
		B.y 		= 	arg(4);
			/* handle no obstacle case... just act like a RMoveTo */

		if (!arg(5)){
			P = (AvdPoint *) NeedPtr(3*sizeof(AvdPoint));
			P[0] = A;
			P[1] = B;
			P[2].x = ENDOFPATH;
			P[2].y = ENDOFPATH;
			acc = Pseudo(P);
			return;
		}


		theList 	= 	(List *) Native(arg(5));
		/* the number of polygons */
		size		=	arg(6); 
		if (argCount >= 7)
			opt		=	arg(7);
		else							/* optimize path or not? */
			opt		=	1;
		if (polylist = RNewPtr (sizeof (polygon) * (size + 1))){ 
			for (it = FirstNode(theList), i = 0 ;  it ; i ++ , it = NextNode(it)){ 
				him = (Obj *) Native(((KNode *)Native(it))->nVal); 
				polylist[i].polyPoints = (AvdPoint *) Native( GetProperty (him, s_points));
				polylist[i].n = GetProperty (him, s_size);
				polylist[i].type = (ubyte) GetProperty (him, s_type);
				polylist[i].info = 0; 
			}
			polylist[i].polyPoints = NULL;
			polylist[i].n = 0;
			polylist[i].type = 0;
			polylist[i].info = 0; 
			acc = Pseudo( getpath(&A,&B,polylist,opt));   
			DisposePtr(polylist);
		}
	}else{	/* just wants interior test */
		size	= (int) GetProperty ((Obj *) Native( arg(3) ), s_size);	
		if (size<3)
			{
			Panic(E_BAD_POLYGON);
			}
		acc 	= (int) ptIntr(&A, (AvdPoint *) Native( GetProperty ((Obj *) Native( arg(3) ), s_points)), size );
	}
}

void KMergePoly(args)
argList;
             
{
   register 	Obj *him;	
	register 	ObjID it;
	List 			*theList;
	AvdPoint 	*Poly;
	polygon		*polylist;
	int 			size, i;



	size = arg(3);


	theList 	= 	(List *) Native(arg(2));
	Poly = (AvdPoint *) Native(arg(1));

	if (polylist = RNewPtr (sizeof (polygon) * (size + 1))){ 
			for (it = FirstNode(theList), i = 0 ;  it ; i ++ , it = NextNode(it)){ 
				him = (Obj *) Native(((KNode *)Native(it))->nVal); 
				polylist[i].polyPoints = (AvdPoint *) Native( GetProperty (him, s_points));
				polylist[i].n = GetProperty (him, s_size);
				polylist[i].type =  (ubyte) GetProperty (him, s_type);
			}
			polylist[i].polyPoints = NULL;
			polylist[i].n = 0;
			polylist[i].type = 0;
		}

		acc = Pseudo(MergePolygons( Poly , polylist));

		for (it = FirstNode(theList), i = 0 ;  it ; i ++ , it = NextNode(it)){ 
				him = (Obj *) Native(((KNode *)Native(it))->nVal); 
				SetProperty(him,s_type,polylist[i].type);
			}

		DisposePtr(polylist);
	


}



global void KListOps(args)
word *args;
{
	register Obj	*him;	
	register ObjID it;

	switch (arg(1)) {
		case	LEachElementDo:		  
			for (it = FirstNode((List *) Native(arg(2))) ;  it ;  
					it = NextNode(it) )	{ 
				him = (Obj *) Native(((KNode *)Native(it))->nVal); 
				InvokeMethod
					((Obj *) Native(((KNode *)Native(it))->nVal), 
						arg(3), argCount - 3 , arg(4), arg(5),  
						arg(6),arg(7), arg(8), arg(9), arg(10)
					);
			}
			break;
		case	LFirstTrue:
			for (it = FirstNode((List *) Native(arg(2))) ;  it ;  
					it = NextNode(it) )	{ 
			  	him = (Obj *) Native(((KNode *)Native(it))->nVal); 
				if	(InvokeMethod 
						((Obj *) Native(((KNode *)Native(it))->nVal), 
							arg(3), argCount - 3 , arg(4), arg(5),  
							arg(6),arg(7), arg(8), arg(9), arg(10)
						)
					){
					acc = Pseudo(him);
					return;
				}
			}
			acc = FALSE;
			break;
		case	LAllTrue:
			for (it = FirstNode((List *) Native(arg(2))) ;  it ;  
					it = NextNode(it) )	{ 
				him = (Obj *) Native(((KNode *)Native(it))->nVal); 
				if	(! (InvokeMethod
							((Obj *) Native(((KNode *)Native(it))->nVal), 
								arg(3), argCount - 3 , arg(4), arg(5),  
								arg(6),arg(7), arg(8), arg(9), arg(10)
							)
						)
					){
					acc = FALSE;
					return;
				}
					
			}
			acc = TRUE;
			break;
	} 
			
}

global void KMemory(args)
word *args;
{
	switch (arg(1)) {
		case	MNeedPtr:
			acc = Pseudo(NeedPtr(arg(2)));
			break;
		case	MNewPtr:
			acc = Pseudo(RNewPtr(arg(2)));
			break;
		case	MDisposePtr:
         if (arg(2) & 1) PError(thisIP,pmsp,E_ODD_HEAP_RETURNED,arg(2),0);
			DisposePtr(Native(arg(2)));
			break;
		case	MCopy:
			memcpy(Native(arg(2)), Native(arg(3)), arg(4));
			break;
		case	MReadWord:
			acc = *(word *)Native(arg(2));
			break;
		case	MWriteWord:
			*((word *)Native(arg(2))) = arg(3);
			break;
		}
}

global void KFileIO(args)
argList;
{
	static DirEntry findFileEntry;

	strptr	buf;
	uint		mode;
	int		fd;

	diskIOCritical = FALSE;

	switch (arg(1)) {
		case fileOpen:
			buf = Native(arg(2));
			mode = arg(3);
			if (mode == F_TRUNC)
				fd = creat(buf, 0);
			else {
				fd = open(buf, O_RDWR);
				if (!criticalError) {
					if (fd == -1 && mode == F_APPEND)
						fd = creat(buf, 0);
					if (fd != -1 && mode == F_APPEND)
						lseek(fd, 0L, LSEEK_END);
					}
				}
			acc = fd;
			break;
		case fileClose:
			/*	reverse sense of return code	*/
			acc = !close(arg(2));
			break;
		case fileRead:
			acc = read(arg(2), Native(arg(3)), arg(4));
			break;
		case fileWrite:
			acc = write(arg(2), Native(arg(3)), arg(4));
			break;
		case fileUnlink:
			acc = unlink(Native(arg(2)));
			break;
		case fileFGets:
			acc = Pseudo(sci_fgets(Native(arg(2)), arg(3), arg(4)));
			break;
		case fileFPuts:
			buf = Native(arg(3));
			acc = write(arg(2), buf, strlen(buf));
			break;
		case fileSeek:
			acc = (int) lseek(arg(2), (long) arg(3), (uchar) arg(4));
			break;
		case fileFindFirst:
			acc = firstfile(Native(arg(2)), arg(4), &findFileEntry);
			if (acc)
				strcpy(Native(arg(3)), findFileEntry.name);
			break;
		case fileFindNext:
			acc = nextfile(&findFileEntry);
			if (acc)
				strcpy(Native(arg(2)), findFileEntry.name);
			break;
		case fileExists:
			if (acc = ((fd = open(Native(arg(2)), O_RDWR)) != -1))
				close(fd);
			break;
		case fileRename:
			acc = rename(Native(arg(2)), Native(arg(3)));
			break;
	}

	diskIOCritical = TRUE;
}


/* a simple bubble sort */
void
bsort(
	SortNode*	theList,
	int			size)
{
	int i,j;
	SortNode tmp;
	for (i=0; i < (size - 1); i++)
		for (j = (size - 1); j>i ; j--)
			if (theList[j].sortKey < theList[j-1].sortKey){ 
				/* swap */
				tmp = theList[j];
				theList[j] = theList[j-1];
				theList[j-1] = tmp;
			}
}


global void KSort(args)
argList;

{
	register 	Obj*			him;
	register 	ObjID			it;
	register		SortNode*	sortArray;

	List 			*theList, *retKList;
	Obj			*retList;
	KNode			*kNode;
	int			size, i;

	theList 		=	(List *) Native(GetProperty ((Obj *) Native(arg(1)),s_elements)) ;
	retList 		=	(Obj *) Native(arg(2));
	size			=	GetProperty ((Obj *) Native(arg(1)), s_size);
	if (size){
		sortArray	= 	NeedPtr(size * sizeof(SortNode));	

		/* set the keys for each SortNode based on passed scoring function */
		for (it = FirstNode(theList), i = 0 ;  it ; i ++ , it = NextNode(it)){ 
			him = (Obj *) Native(((KNode *)Native(it))->nVal); 
			sortArray[i].sortObject = him;
			sortArray[i].sortKey = InvokeMethod (him, s_perform, 1, arg(3));
		}

		/* the actual sort */
		bsort(sortArray, size);

		retKList = NeedPtr(sizeof(List));
		InitList(retKList);

		/* stuff objects into return List */
		for (i = 0; i < size; i++){
			kNode = (KNode *) NeedPtr(sizeof(KNode));	
			kNode->nVal = (word)Pseudo( sortArray[i].sortObject );
			AddToEnd(retKList, (word)Pseudo( kNode ), Pseudo( sortArray[i].sortObject) ); 	
		}

		SetProperty(retList, s_elements, (word) Pseudo( retKList ));
		SetProperty(retList, s_size, size);

		DisposePtr(sortArray);
	}
}


global void KPalVary(args)
argList;
{
	Handle BTargetPalette;
   int   ticks;
   int   range;
   /* 
   input:
      arg1 = function number 
      Other args depend on function number.
      When another picture is drawn the palette will
      be automatically varied based upon the time and
      a target palette which is named xxx.pal where
      the picture to be drawn is xxx.p56.

   */

	switch (arg(1)) 
      {
		case PALVARYSTART:

         /* input:
               arg2 = target palette number (required)
               arg3 = time to shift palette in seconds (required)
                      if time = zero then palette changed immediately 
                      to destination percent
               arg4 = target percent for shift (not required unless arg5 wanted)
                      If not present 64 assumed to be target percent
               arg5 = delta percent for each palette change (not required)
                      If not present 1 assumed
         */
         if (!palVaryOn)
            {
				targetPalette = ResLoad(RES_MEM, sizeof(RPalette));
				startPalette = ResLoad(RES_MEM, sizeof(RPalette));
				newPalette = ResLoad(RES_MEM, sizeof(RPalette));
				palVaryOn = TRUE;
            paletteDir = 1;
            paletteStop = 64;
            paletteRes = arg(2);
            paletteTime = arg(3);
            /* load palette */
	         if ((BTargetPalette = ResLoad(RES_PAL, paletteRes)) != 0)
               {
               /* set up defaults for calculated palette */
					CopyNew2OldPalette((RPalette far *) *targetPalette,(danPalette far *) *BTargetPalette);
		         PaletteShell((RPalette far *) *targetPalette);
					ResUnLoad(RES_PAL, paletteRes);

               // use the current palette as the start palette 
					// Careful! it may not be correct!
					// sysPalette could already be night? or ???

					FarMemcpy(*startPalette,&sysPalette,sizeof(RPalette));

               /* get memory for calculated palette */
               /* set up defaults for calculated palette */
		         PaletteShell((RPalette far *) *newPalette);

               /* calculate timmer interrupt */
               /* assume max of 64 variations */
               /* paletteShift goes from 0 to 63 */
               palettePercent = 1;
               ticks = (paletteTime*60 + 32)/64;
               /* if target percent given */
               if (argCount > 3) paletteStop = arg(4);
               /* if delta percent given */
               if (argCount > 4) paletteDir = arg(5);
               /* if time == 0 make the palette shift in one call */
               /* if EGA make the palette shift in one call */
               if ((ticks == 0) || (NumberColors == 16))
                  {
                  paletteDir = paletteStop;
                  ticks = 1;
                  }
               /* start server */
	            InstallServer(PaletteServer,ticks);
               /* the server should now do all the work */
               acc = 1;
               }
            }
         else
            {
            /* not finished with previous palette shift */
            acc = 0;
            return;
            }
			break;

		case PALVARYREVERSE:

         /* input:
               target palette will be orginal start palette and start palette
               will be orignal target palette

               arg2 = time to shift palette in seconds (not required unless arg3 wanted)
                      if time = zero then palette changed immediately 
                      to destination percent
               arg3 = target percent for shift (not required unless arg4 wanted)
                      If not present 0 assumed to be target percent
               arg4 = delta percent for each palette change (not required)
                      If not present previous value assumed
         */
         if (palVaryOn)
            {
				if(palettePercent > 64)
					palettePercent = 64;

            /* stop palette server */
            KillPalServer();
            paletteStop = 0;
            if (argCount > 1) paletteTime = arg(2);
            if (argCount > 2) paletteStop = arg(3);
            if (argCount > 3) 
               paletteDir = -arg(4);
            else
               paletteDir = -paletteDir;

            /* calculate timmer interrupt */
            /* assume max of 64 variations */
            ticks = (paletteTime*60 + 32)/64;

            /* if time == 0 make the palette shift in one call */
            /* if EGA make the palette shift in one call */
            if ((ticks == 0) || (NumberColors == 16))
               {
               paletteDir = paletteStop - palettePercent;
               ticks = 1;
               }
            /* start server */
	         InstallServer(PaletteServer,ticks);
            /* the server should now do all the work */
            if (paletteDir > 0)
               acc = palettePercent;
            else
               acc = -palettePercent;
            }
         else
            {
            /* There is no target palette */
            /* When palettePercent went to zero, the palette
               was unloaded and the server released. */
            acc = 0;
            }
			break;

		case PALVARYINFO:

         /* return percent of palette change */
         /* 64 = 100% */
         if (paletteDir >= 0)
            acc = palettePercent;
         else
            acc = -palettePercent;
			break;


		case PALVARYNEWTIME:			// Change the time for reaching target

         /* input:
               arg2 = time to finish (required)
         */
			if(palVaryOn)
				{
				paletteTime = arg(2);
				range = abs(paletteStop - palettePercent);
            if (range)
               {
				   ticks = (paletteTime * 60 + range/2)/range;
         	   KillPalServer();
	            InstallServer(PaletteServer,ticks);
               }
			   }
			break;

		case PALVARYKILL:

         /* stop palette change */
         KillPalServer();
			// ResUnLoad(RES_PAL, (uword) paletteRes);
			ResUnLoad(RES_MEM, (uword) startPalette);
			ResUnLoad(RES_MEM, (uword) newPalette);
			ResUnLoad(RES_MEM, (uword) targetPalette);
			palVaryOn = FALSE;
			break;

		case PALVARYTARGET:

         /* input:
               arg2 = target palette number (required)
         */
         /* insert custom colors into target palette */
         /* Note: custom colors are inserted into start
            palette by RSetPalette */
         if (palVaryOn)
            {
	         if ((BTargetPalette = ResLoad(RES_PAL, arg(2))) != 0)
               {
					CopyNew2OldPalette((RPalette far *) *newPalette,(danPalette far *) *BTargetPalette);
					InsertPalette((RPalette far *) *newPalette,(RPalette far *) *targetPalette);
					ResUnLoad(RES_PAL, (uword) arg(2));
      			PaletteUpdate(DOSETCLUT,0);		// updates sysPalette & sets clut
               }
            }
         if (paletteDir >= 0)
            acc = palettePercent;
         else
            acc = -palettePercent;
			break;

		case PALVARYPAUSE:

         /* input:
               arg2 = TRUE or FALSE (required)
               TRUE:  Pause the palette change.
               FALSE  Restart the palette change.
         */
         /* Note:
               If PalVary is paused n times with TRUE it will require
               n calls to PalVary pause with FALSE to restart the palette
               changing.
        */
         if (palVaryOn)
            {
            if (arg(2)) 
               ++palVaryPause;
            else
               if (palVaryPause) 
                  --palVaryPause;
            }
			break;

      }
}


global void KShowMovie(args)
argList;

{
	char * fileName;
	Handle bufferHandle;
	int handle;
	char movieName[80];

	fileName = (char *) arg(1);
	if(movieDir) {
		strcpy(movieName,movieDir);
		strcat(movieName,"\\");
		strcat(movieName,fileName);
		}
	else
		strcpy(movieName,fileName);

	bufferHandle = ResLoad(RES_MEM, 64200U);	// a full screen

	handle = open(movieName,0);

	if(handle == -1) {
		DoAlert("Unable to Open Movie File");
		return;
	}

	RunMovie(handle,bufferHandle,arg(2)); 	// arg2 = frame rate
	close(handle);
	
	picNotValid = FALSE;
	ResUnLoad(RES_MEM,(uword) bufferHandle);
}

global void KSetVideoMode(args)

argList; 

{

	int mode = (int) arg(1);
	char far * ptr;
	int k;

	char lastMode = currentVideoMode;

	if(currentVideoMode == (char) mode)
		return;

	switch(mode) {

		case 1:			// Set Video Mode to ModeX
		case 2:

			RHideCursor();			// cursor MUST be off in modeX

			SetVideoMode(mode);
			currentVideoMode = (char) mode;

			if(mode == 2)
				RShowCursor();		// cursor can now be on
					
			break;

		case 0:

			RHideCursor();
				
			SetVideoMode(0);

			// set the palette back to what it was when we changed over
			workPalette = ResLoad(RES_MEM, sizeof(RPalette));
			ptr = ((char far *) *workPalette) + 0x104;
			for(k = 0; k < 256; k++) {
				*ptr++ = 1;
				*ptr++ = 0;
				*ptr++ = 0;
				*ptr++ = 0;
			}

			SetCLUT((RPalette far *) *workPalette,TRUE);		// force clear of palette
			ResUnLoad(RES_MEM, (uword) workPalette);
			SetCLUT((RPalette far *) &sysPalette,TRUE);		// put palette back to sysPalette

			picNotValid = TRUE;

			RShowCursor();		// cursor can now be on
			if(lastMode == 1)
				RShowCursor();		// Show it once more (ModeXV forced it off)

			currentVideoMode = 0;

			break;

		default:
			DoAlert("Invalid Video Mode Request");

	}

}

void
KSetQuitStr(
	argList)
{
	strncpy(quitStr, Native(arg(1)), MAX_QUITSTR-1);
}

global void KDbugStr(args)
argList;
{
	MonoStr((char far *)Native(arg(1)));
}
