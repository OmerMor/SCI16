/*
** ACTOR - Routines affecting animated (or stationary) actors
**		 drawn in the picture window
*
*/

#include "selector.h"

#include	"kernel.h"
#include	"pmachine.h"
#include	"animate.h"
#include	"cels.h"
#include	"picture.h"
#include	"list.h"
#include	"memmgr.h"
#include	"event.h"
#include	"dialog.h"
#include	"object.h"
#include	"resource.h"
#include "sciglobs.h"
#include	"stdio.h"
#include	"graph.h"
#include "scale.h"
#include	"vm_defs.h"
#include	"start.h"
#include	"errmsg.h"

static void	near	SortCast(Obj **, word *, word *, word);
static void	near	ReDoStopped(Obj **, byte *, word);
static void near  doScale(Obj *,Handle, word, word, Handle *, word, word);

#define	PLENTYROOM	1	/* only applies to this module */
#define	SORTONZTOO		/* only applies to this module */
#define	NOSAVEBITS -1

/* a "no update" actor has a clean underBits at lastSeen */


typedef struct {
	Node		aLink;
	word		v;
	word		l;
	word		c;
	word		p;
	word		pal;
	Handle	u;
	RRect		r;
	Obj*		this;
	word  	isScaled;
} AniObj;

global void Animate(cast, doit)		/* update all actors in cast */
List *cast;
word doit;
{
	AniObj *dup;
	register Obj *him;
	word	celNum, loopNum, castSize;
	ObjID	it;
	RGrafPort *oldPort;
	RRect ur;
	Handle view;
	word signal, i;
	word showAll = picNotValid;
	Obj *castID[200];
	word	castY[200];
   word 	castPri[200];
	byte	castShow[200];

	word scale = 0,newXDim, newYDim, oldXDim, oldYDim,
        newXOff, newYOff, oldXOff, oldYOff;

	word theScaleX, theScaleY;

	if (!cast) {
		/* No castList -- just show picture (if necessary) */
		DisposeLastCast();
		if (picNotValid){
			ShowPic(showMap, showStyle);
			picNotValid = 0;
		}
		return;
	}

	/* invoke doit: method for each member of the cast */
	/* unless thay are identified as a static view */
	if (doit)
		for (it = FirstNode(cast) ;  it ;  it = NextNode(it) ) {
			if (globalVar[g_fastCast]) return;
			him = (Obj *) Native(((KNode *)Native(it))->nVal);
	
			if (!(IndexedProp(him, actSignal) & staticView))
				InvokeMethod(him, s_doit, 0);
			}


	RGetPort(&oldPort);
	RSetPort((RGrafPort *) picWind);

	/* dispose the old castList (if any) and get a new one */
	DisposeLastCast();
	lastCast = (List *) NeedPtr(sizeof(List));
	InitList(lastCast);


	/* the screen is clean except for stopped actors */
	/* validate LOOP/CEL and update nowSeen for ALL actors in the cast */
	/* set castID array for all subsequent access to the cast */
	castSize = 0;
	for (it = FirstNode(cast) ;  it ;  it = NextNode(it) ) {
		him = (Obj *) Native(((KNode *)Native(it))->nVal);

		/* save their object IDs in castID array */
		castID[castSize] = him;

		/* save their y positions in castY array */
		castY[castSize] = IndexedProp(him, actY);

		castPri[castSize] = IndexedProp(him, actPri);
		if (-1 == castPri[castSize])
			castPri[castSize] = CoordPri(castY[i]);

		/* zero out every show (no matter what sort does) */
		castShow[castSize] = 0;

		++castSize;

		signal = IndexedProp(him, actSignal);
	/* this handle will be valid for a while */
		theViewNum = IndexedProp(him, actView);
		view = ResLoad(RES_VIEW, theViewNum);

	/* check valid loop and cel */
		loopNum = IndexedProp(him, actLoop);
		theLoopNum = loopNum;

		if (loopNum >= (word) GetNumLoops(view)){
			loopNum = 0;
			IndexedProp(him, actLoop) = loopNum;
		}

		celNum = IndexedProp(him, actCel);
		theCelNum = celNum;
		if (celNum >= GetNumCels(view, loopNum)){
			celNum = 0;
			IndexedProp(him, actCel) = celNum;
		}
	/* if we are a scaled view, don't obtain new rectangle */
		scale = GetProperty(him,s_vm_signal);


		if( !(scale & VM_SCALABLE)) 
			GetCelRect(view, loopNum, celNum,
				IndexedProp(him, actX),
				IndexedProp(him, actY),
				IndexedProp(him, actZ),
				(RRect *) IndexedPropAddr(him, actNS));



		/* if fixPriority bit in signal is clear we get priority for this line */
		if (!(signal & FIXPRI))
			IndexedProp(him, actPri) = CoordPri(IndexedProp(him, actY));


	/* deal with the "signal" bits affecting stop updated actors in cast */
	/* turn off invalid bits for either condition */
		if (signal & NOUPDATE){

			/* STOPPED actors only care about STARTUPD, FORCEUPD and HIDE/HIDDEN bits */
			if ((signal & (STARTUPD | FORCEUPD)) 
				|| ((signal & HIDE) && !(signal & HIDDEN)) 
					|| (!(signal & HIDE) && (signal & HIDDEN))
						|| (signal & VIEWADDED)){
				++showAll;
			}
			signal &= (~ STOPUPD);

		} else {

			/* nonSTOPPED actors care about being stopped or added */
			if (signal & STOPUPD
				|| (signal & VIEWADDED)){
				++showAll;
			}
			signal &= ~FORCEUPD;
		}

		/* update changes to signal */
		IndexedProp(him, actSignal) = signal;
	}

	/* sort the actor list by y coord */
	SortCast(castID, castY, castPri, castSize);

/* if showAll is true we must save a "clean" underbits of all stopped actors */
/* any newly stopped actors are treated equally */
	if (showAll){
		ReDoStopped(castID, castShow, castSize);
	}


/* draw nonSTOPPED nonHIDDEN cast from front to back of sorted list (forward) */
	for (i = 0 ; i < castSize; i++){
		him = castID[i];
		signal = IndexedProp(him, actSignal);

	/* this handle will be valid for a while */
		theViewNum = IndexedProp(him, actView);
		view = ResLoad(RES_VIEW, theViewNum);

	/* all stopped actors have already been drawn */
	/* a HIDE actor is not drawn */
	/* a VIEWADDED is not drawn */
		if (!(signal & (VIEWADDED | NOUPDATE | HIDE))){

			ResLock(RES_VIEW, theViewNum, TRUE);

			scale = GetProperty(him,s_vm_signal);

  /* if we are a scaled view, perform scaling transform */
			if((scale & VM_SCALABLE))
				{
				loopNum = IndexedProp(him, actLoop);
				theLoopNum = loopNum;
				celNum = IndexedProp(him, actCel);
				theCelNum = celNum;
				oldXDim = GetCelWide(view,loopNum,celNum);
				oldYDim = GetCelHigh(view,loopNum,celNum);
				oldXOff = GetCelXOff(view,loopNum,celNum);  //Added MLP for offset scale
				oldYOff = GetCelYOff(view,loopNum,celNum);
  /* check if we are scaling here or under SCI control */
				if(! (scale & VM_AUTO_SCALE))
					{
						theScaleX = GetProperty(him,s_scaleX);
						theScaleY = GetProperty(him,s_scaleY);
					}
				else
					{
						// get the scale factor
						GetNewScaleFactors(him, oldYDim, &theScaleX, &theScaleY );
						SetProperty(him,s_scaleX, theScaleX);
						SetProperty(him,s_scaleY, theScaleY);
					}

         // determine the new cell height and width along with x & y offsets
	 	 	vm_size(oldXDim,oldYDim,oldXOff,oldYOff,theScaleX,theScaleY,
                 &newXDim,&newYDim,&newXOff,&newYOff
			 	);		
	  
	/* for the moment set the views dimensions to reflect the scaling op */

            /* set the scaled cel dimentions and offsets */
		  		SetNewDim(view,loopNum, celNum, newXDim, newYDim, newXOff, newYOff); 
				GetCelRect(view, loopNum, celNum,
					IndexedProp(him, actX),
					IndexedProp(him, actY),
					IndexedProp(him, actZ),
					(RRect *) IndexedPropAddr(him, actNS)
				);

				IndexedProp(him, actUB) =
				Pseudo(SaveBits((RRect *) IndexedPropAddr(him, actNS), PMAP | VMAP | CMAP));


	/* perform the actual drawcel of the scaled view */

				theLoopNum = IndexedProp(him, actLoop);
				theCelNum = IndexedProp(him, actCel);
	 		  	ScaleDrawCel(view, theLoopNum,theCelNum,
					(RRect *) IndexedPropAddr(him, actNS),
					IndexedProp(him, actPri),
					oldXDim
				);
			  
  	/* return the view's dimensions back to normal */
 				SetNewDim(view,loopNum,celNum,oldXDim, oldYDim, oldXOff, oldYOff);

				}
			else
				{
				IndexedProp(him, actUB) =
				Pseudo(SaveBits((RRect *) IndexedPropAddr(him, actNS), PMAP | VMAP | CMAP));

				DrawCel(view, IndexedProp(him, actLoop), IndexedProp(him, actCel), 
					(RRect *) IndexedPropAddr(him, actNS),
					IndexedProp(him, actPri)
				);

				}


			ResLock(RES_VIEW, theViewNum, FALSE);
			castShow[i] = 1;

		/* this actor was drawn so we must unHIDE him */
			if (signal & HIDDEN && !(signal & HIDE)){
				signal &= ~HIDDEN;
				/* update changes to signal */
				IndexedProp(him, actSignal) = signal;
			}

		/* add an entry to ReAnimate list */
			dup = (AniObj *) NeedPtr(sizeof(AniObj));
			dup->this = him;
			dup->v = IndexedProp(him, actView);
			dup->l = IndexedProp(him, actLoop);
			dup->c = IndexedProp(him, actCel);
			dup->p = IndexedProp(him, actPri);
			dup->isScaled = scale ;
			RCopyRect( (RRect *) IndexedPropAddr(him, actNS), &dup->r);
			AddToEnd(lastCast, Pseudo(dup)); 
		}
	}

/* do we show the screen here */
	if (picNotValid){
		ShowPic(showMap, showStyle);
		picNotValid = 0;
	}

/* now once more through the list to show the bits we have drawn */
	for (i = 0; i < castSize; i++){
		him = castID[i];
		signal = IndexedProp(him, actSignal);

	/* "stopped" actors don't need showing
	**  unless a newStop has been done */
	/* hidden actors don't need it either */
		if ((castShow[i]) || (!(signal & HIDDEN) && (!(signal & NOUPDATE) || showAll))){

			/* update visual screen */
			if (CSectRect((RRect *) IndexedPropAddr(him, actLS),
				(RRect *) IndexedPropAddr(him, actNS), &ur)){

				/* show the union */
				RUnionRect((RRect *) IndexedPropAddr(him, actLS),
						(RRect *) IndexedPropAddr(him, actNS), &ur);
	
				ShowBits(&ur, showMap); 
			} else {
				/* show each separately */
				ShowBits((RRect *) IndexedPropAddr(him, actNS), showMap);
				ShowBits((RRect *) IndexedPropAddr(him, actLS), showMap);
			}

			
			RCopyRect((RRect *) IndexedPropAddr(him, actNS),
					(RRect *) IndexedPropAddr(him, actLS));

			/* now reflect hidden status */
			if (signal & HIDE && !(signal & HIDDEN)){
				signal |= HIDDEN;
			}
		}

		/* update any changes */
		IndexedProp(him, actSignal) = signal;
	}

/* now after all our hard work we are going to erase the entire list */
/* except for NOUPDATE actors */

	for (i = castSize - 1; i >=0; i--){
		him = castID[i];

	/* erase him from background */

		signal = IndexedProp(him, actSignal);
		if (!((signal & NOUPDATE) || (signal & HIDDEN))){

			RestoreBits((Handle) Native(IndexedProp(him, actUB)));
			IndexedProp(him, actUB) = 0;
		}
		/* invoke the delete method of certain objects */
		if (signal & delObj){
			InvokeMethod(him, s_delete, 0);
		}			
	}


/* restore current port */
	RSetPort(oldPort);
}

/* general routine to perform scaling as in main animate loop,
	to be called from the likes of Reanimate and ReDoStopped */

static void near doScale(vId, theView, lnum, cnum, unders,theScale,saveUnders)
	Obj *vId;
	Handle theView;
	word lnum;
	word cnum;
	Handle	*unders;
	word theScale;
	word saveUnders;
{

word theScaleX, theScaleY, oldXDim, oldYDim, oldXOff, oldYOff,
     newXDim, newYDim, newXOff, newYOff;

oldXDim = GetCelWide(theView,lnum,cnum);
oldYDim = GetCelHigh(theView,lnum,cnum);
oldXOff = GetCelXOff(theView,lnum,cnum);    //Added MLP for offset scale
oldYOff = GetCelYOff(theView,lnum,cnum);


if(! (theScale & VM_AUTO_SCALE))
		{
		  	theScaleX = GetProperty(vId,s_scaleX);
			theScaleY = GetProperty(vId,s_scaleY);			
		}
	else
		{
			// get the scale factors
			GetNewScaleFactors(vId, oldYDim, &theScaleX, &theScaleY );
			SetProperty(vId,s_scaleX, theScaleX);
			SetProperty(vId,s_scaleY, theScaleY);
		}

   // determine the new cell height and width along with x & y offsets
	vm_size(oldXDim,oldYDim,oldXOff,oldYOff,theScaleX,theScaleY,
           &newXDim,&newYDim,&newXOff,&newYOff
	);

   /* set the scaled cel dimentions and offsets */
	SetNewDim(theView,lnum, cnum, newXDim, newYDim, newXOff, newYOff);
	GetCelRect(theView, lnum, cnum,
		IndexedProp(vId, actX),
		IndexedProp(vId, actY),
		IndexedProp(vId, actZ),
		(RRect *) IndexedPropAddr(vId, actNS)
	);

	if(saveUnders != NULL )
	*unders =
(Handle)	Pseudo(SaveBits((RRect *) IndexedPropAddr(vId, actNS), VMAP | PMAP |CMAP));


	ScaleDrawCel(theView, lnum, cnum,
		(RRect *) IndexedPropAddr(vId, actNS),
		IndexedProp(vId, actPri),
		oldXDim
	);
   /* reset the non scaled cel dimentions and offsets */	
 	SetNewDim(theView,lnum,cnum,oldXDim, oldYDim, oldXOff, oldYOff);
}	

global void AddToPic(cast)		/* Sort and draw list of adds to the picture */
List *cast;
{
	register Obj *him;
	ObjID	it;
	RGrafPort *oldPort;
	RRect r;
	Handle view;
	word signal, i, priority, top, castSize, scale;
	Obj *castID[200];
	word	castY[200];
   word	castPri[200];


	/* NOTE: this routinemust use the slower GETPROPERTY access method
	** because the addToPic list may contain either PicViews or Actors
	*/

	if (cast){
		RGetPort(&oldPort);
		RSetPort((RGrafPort *) picWind);

		/* set castID array for all subsequent access to the cast */
		castSize = 0;
		for (it = FirstNode(cast) ;  it ;  it = NextNode(it) ) {
			him = (Obj *) Native(((KNode *)Native(it))->nVal);
	
			/* save their object IDs in castID array */
			castID[castSize] = him;
	
			/* save their y positions in castY array */
			castY[castSize] = GetProperty(him, s_y);
			castPri[castSize] = GetProperty(him, s_priority);
			if (-1 == castPri[castSize])
				castPri[castSize] = CoordPri(castY[i]);
			++castSize;
		}

		/* sort the actor list by y coord */
		SortCast(castID, castY, castPri, castSize);

		/* now draw the objects from back to front */
		for (i = 0; i < castSize; i++){
			him = castID[i];
			signal = GetProperty(him, s_signal);

			/* get the cel rectangle */

			scale = GetProperty(him, s_vm_signal);

			theViewNum = GetProperty(him, s_view);
			view = (Handle) ResLoad(RES_VIEW, theViewNum);
			ResLock(RES_VIEW, theViewNum, TRUE);

			if(( scale & VM_SCALABLE))

				{

					/* get a priority */
					if (-1 == (priority = GetProperty(him, s_priority)))
						priority = CoordPri(castY[i]);

					doScale(him, (Handle) view, (word) IndexedProp(him,actLoop),
							(word) IndexedProp(him, actCel),  NULL,
							(word) scale, (word) FALSE
					);

				}
			else
				{

					GetCelRect(
						view, 
						GetProperty(him, s_loop), 
						GetProperty(him, s_cel),
						GetProperty(him, s_x),
						GetProperty(him, s_y),
						GetProperty(him, s_z),
						&r
					);
	
					/* get a priority */
					if (-1 == (priority = GetProperty(him, s_priority)))
						priority = CoordPri(castY[i]);

					DrawCel((Handle) view, GetProperty(him, s_loop),
						GetProperty(him, s_cel),
						&r,
						priority
					);


		  		}

			ResLock(RES_VIEW, theViewNum, FALSE);

			/* if ignrAct bit is not set we draw a control box */
			if (!(signal & ignrAct)){

				/* compute for priority that he is */
				top = PriCoord(priority) - 1;

				/* adjust top accordingly */
				if (top < r.top)
					top = r.top;

				if (top >= r.bottom)
					top = r.bottom - 1;

				r.top = top;
				RFillRect(&r, CMAP, 0, 0, 15);
			}
		}
	}
}


global void DisposeLastCast()
/* abandon last cast if there is one */
{
	register	ObjID	dup;

	if (!lastCast)
		return;

	for (dup = FirstNode(lastCast) ;  dup ;  dup = FirstNode(lastCast) ) {
		DeleteNode(lastCast, dup);
		DisposePtr(Native(dup));
		}

	DisposePtr(lastCast);
	lastCast = NULL;
}




static void near SortCast(cast, casty, castpri, size)	/* sort cast array by y property */
Obj *cast[];
word casty[];
word castpri[];
int size;
{
	word i, j, swapped = 1;
	register Obj *tmp;
	register word tmpword;

	for (j = (size - 1) ; j > 0 ; j--){
		swapped = 0;
		for (i = 0; i < j; i++){
			/* base it on y position */

			if (castpri[i + 1] < castpri[i]
				|| (castpri[i + 1] == castpri[i]
					&& casty[i + 1] < casty[i]	
#ifdef SORTONZTOO
					|| (casty[i + 1] == casty[i]
						&&	GetProperty(cast[i + 1], s_z) < GetProperty(cast[i], s_z)
					)
#endif
				)
			){
				tmp = cast[i];
				cast[i] = cast[i + 1];
				cast[i + 1] = tmp;

				/* exchange y positions also */
				tmpword = casty[i];
				casty[i] = casty[i + 1];
				casty[i + 1] = tmpword;

				/* exchange pri positions also */
				tmpword = castpri[i];
				castpri[i] = castpri[i + 1];
				castpri[i + 1] = tmpword;

				swapped = 1;
			}
		}
		if (!swapped)
			break;
	}
}

static void near ReDoStopped(cast, castShow, size)	/* redo all "stopped" actors */
Obj *cast[];
byte castShow[];
int size;
{
	word					i, top;
	register Obj	*him;
	word					signal;
	Handle					view, scaledUnders ;
	RRect					r;
	word					scale,loopNum,celNum;
	ObjID					underBits;
#ifdef PLENTYROOM
	word					whichMaps;
#endif


	RBeginUpdate(picWind); /* save windows so that stopUpd'd actor don't show through */
/* to ReDo stopped actors, we must TOSS all backgrounds in reverse */
	for (i = size - 1; i >= 0 ; i--){
		him = cast[i];
		signal = IndexedProp(him, actSignal);
		if (signal & NOUPDATE) {
			if (!(signal & HIDDEN)) {
				underBits = IndexedProp(him, actUB);
				if (picNotValid == 1) {	/* a new picture */
					if (underBits)
						UnloadBits((Handle) Native(underBits));

				} else {
					RestoreBits((Handle) Native(underBits));
					castShow[i] = 1;
				}

		 		IndexedProp(him, actUB) = 0;
			}

			/* now is when we catch START updates and HIDE/SHOWs */
			if (signal & FORCEUPD)
				signal &= ~FORCEUPD;

			/* logic must respect transition to UPDATING */
			if (signal & STARTUPD){
				signal &= (~(STARTUPD | NOUPDATE));
			}

		} else {

		/* an updating actor that may want to stop */
			if (signal & STOPUPD){
				signal = (signal & ~STOPUPD) |  NOUPDATE;
			}
		}

		/* update signal property */
		IndexedProp(him, actSignal) = signal;
	}

	/* the screen is absolutely clean at this point */
	/* Draw in ANY actors with a view added property and condition signal */
	for (i = 0; i < size ; i++){
		him = cast[i];
		signal = IndexedProp(him, actSignal);
		if (signal & VIEWADDED)
         {
         theViewNum = IndexedProp(him, actView);
			view =  ResLoad(RES_VIEW,theViewNum);
			ResLock(RES_VIEW, theViewNum, TRUE);

			scale = GetProperty(him,s_vm_signal);

	/* if the view is scaled, react accordingly */
			if((scale & VM_SCALABLE))
				doScale(him, (Handle) view, (word) IndexedProp(him,actLoop),
							(word) IndexedProp(him, actCel),  (word) NULL,
							(word) scale, (word) NULL
				);
			else
				{

				DrawCel((Handle) view, IndexedProp(him, actLoop),
					IndexedProp(him, actCel),
					(RRect *) IndexedPropAddr(him, actNS),
					IndexedProp(him, actPri)
					);
				}
		  	castShow[i] = 1;

			ResLock(RES_VIEW, theViewNum, FALSE);

			signal &= (~(NOUPDATE | STOPUPD | STARTUPD | FORCEUPD));
			RCopyRect( (RRect *) IndexedPropAddr(him, actNS), &r);

			/* if ignrAct bit is not set we draw a control box */
			if (!(signal & ignrAct)){

				/* compute for4 priority that he is */
				top = PriCoord(IndexedProp(him, actPri)) - 1;

				/* adjust top accordingly */
				if (top < r.top)
					top = r.top;

				if (top >= r.bottom)
					top = r.bottom - 1;

				r.top = top;
				RFillRect(&r, CMAP, 0, 0, 15);
			}
		}
		IndexedProp(him, actSignal) =  signal;
	}

/* now save a clean copy of underbits for NOUPDATE / nonHIDDEN actors*/
	for (i = 0; i < size; i++){
		him = cast[i];
		if (NOUPDATE & (signal = IndexedProp(him, actSignal))){

			/* make HIDE/SHOW decision first */
			if (signal & HIDE)
				signal |= HIDDEN;
			else
				signal &= ~HIDDEN;
			IndexedProp(him, actSignal) =  signal;
			if (!(signal & HIDDEN) ){
				scale = GetProperty(him,s_vm_signal);
				
#ifdef PLENTYROOM
				whichMaps = PMAP | VMAP;
				if (!(signal & ignrAct) )
					whichMaps |= CMAP;
				
/* If scaled, we must first scale the actor before saving underbits */
					if((scale & VM_SCALABLE))
						{
						
							theViewNum = IndexedProp(him, actView);
							view =  ResLoad(RES_VIEW,theViewNum);
							loopNum = IndexedProp(him, actLoop);
							celNum = IndexedProp(him, actCel);
		 					doScale(him, (Handle) view, (word) loopNum,
							(word) celNum,  (Handle *) &scaledUnders,
							(word) scale, 	(word) TRUE
							);

							IndexedProp(him, actUB) = Pseudo((Handle) scaledUnders);
						}
					else
						IndexedProp(him, actUB) =
						Pseudo(SaveBits((RRect *) IndexedPropAddr(him, actNS),
						whichMaps));					 
#else
				if((scale & VM_SCALABLE))
						{
						
							theViewNum = IndexedProp(him, actView);
							view =  ResLoad(RES_VIEW,theViewNum);
							loopNum = IndexedProp(him, actLoop);
							celNum = IndexedProp(him, actCel);
		 					doScale(him, (Handle) view, (word) loopNum,
							(word) celNum,  (Handle *) &scaledUnders,
							(word) scale, 	(word) TRUE
							);

							IndexedProp(him, actUB) = Pseudo((Handle) scaledUnders);

						}
					else
						IndexedProp(him, actUB) =
						Pseudo(SaveBits((RRect *) IndexedPropAddr(him, actNS),
						VMAP | PMAP | CMAP));
#endif
			}
		}
	}

	/* now redraw them at nowSeen from head to tail of list */
	for (i = 0; i < size; i++){
		him = cast[i];
		signal = IndexedProp(him, actSignal);
		if (signal & NOUPDATE && !(signal & HIDE))
         {
         theViewNum = IndexedProp(him, actView);
			view =  ResLoad(RES_VIEW,theViewNum);
			ResLock(RES_VIEW, theViewNum, TRUE);

			/* get nowSeen into local and */
			RCopyRect( (RRect *) IndexedPropAddr(him, actNS), &r);
/*			RCopyRect( &r, (RRect *) IndexedPropAddr(him, actLS)); */

		/* draw it */

		/* handle scaled views accordingly */
			scale = GetProperty(him,s_vm_signal);

			if((scale & VM_SCALABLE))
				{	
					doScale(him,(Handle) view, IndexedProp(him,actLoop),
							IndexedProp(him, actCel), (word) NULL ,
							scale, (word) FALSE
					);
/*					IndexedProp(him, actUB) = Pseudo(scaledUnders); */
				}
			else
				{

				DrawCel((Handle) view, IndexedProp(him, actLoop),
					IndexedProp(him, actCel),
					(RRect *) IndexedPropAddr(him, actNS),
					IndexedProp(him, actPri)
					);
				 }
			castShow[i] = 1;

			ResLock(RES_VIEW, theViewNum, FALSE);

		/* if ignrAct bit is not set we draw a control box */
			if (!(signal & ignrAct)){

				/* compute for4 priority that he is */
				top = PriCoord(IndexedProp(him, actPri)) - 1;

				/* adjust top accordingly */
				if (top < r.top)
					top = r.top;

				if (top >= r.bottom)
					top = r.bottom - 1;

				r.top = top;
				RFillRect(&r, CMAP, 0, 0, 15);
			}
		}
		IndexedProp(him, actSignal) =  signal;
	}
	REndUpdate(picWind);	/* redraw windows over already drawn stopUpd'ers */
}

global void ReAnimate(badRect)		/* redraw cast from last cast list */
RRect *badRect;
{
	RGrafPort	*oldPort;
	AniObj	*dupPtr;
	Handle	view;
	RRect		bRect;
	ObjID		dup;

	Obj	*him;
	
	/* bad rect may be in Local Coords of another window (port),
	we must make them local to this port */
	RCopyRect(badRect, &bRect);

	RLocalToGlobal((RPoint *) &(bRect.top));
	RLocalToGlobal((RPoint *) &(bRect.bottom));
	RGetPort(&oldPort);
	RSetPort((RGrafPort *) picWind);
	RGlobalToLocal((RPoint *) &(bRect.top));
	RGlobalToLocal((RPoint *) &(bRect.bottom));

	/* this is valid only if we have a lastCast */
	if (lastCast){
		for (dup = FirstNode(lastCast);  dup;  dup = NextNode(dup)) {
			dupPtr = (AniObj *) Native(dup);
	
	/* handle scaled views accordingly */

			view = ResLoad(RES_VIEW, dupPtr->v);
			ResLock(RES_VIEW, dupPtr->v, TRUE);

			if((dupPtr->isScaled & VM_SCALABLE))
				{
					him = dupPtr->this;
					doScale(him, view, dupPtr->l, dupPtr->c,&dupPtr->u, dupPtr->isScaled, (word) TRUE);
				}
	
				else
				{	  
					dupPtr->u = SaveBits(&dupPtr->r, VMAP|PMAP);
					DrawCel(view, dupPtr->l, dupPtr->c, &dupPtr->r, dupPtr->p);
				}

			ResLock(RES_VIEW, dupPtr->v, FALSE);

		}

		/* Show the disturbed rectangle */
		ShowBits(&bRect, showMap); 

		/* now erase them again */
		for (dup = LastNode(lastCast);  dup;  dup = PrevNode(dup)) {
			dupPtr = (AniObj *) Native(dup);
			RestoreBits(dupPtr->u);
		}
	} else {
		/* Show the disturbed rectangle */
		ShowBits(&bRect, showMap);
	}


	RSetPort(oldPort);
}

