// Window manager - Really neat stuff

#include	"animate.h"
#include	"dialog.h"
#include	"errmsg.h"
#include	"event.h"
#include	"graph.h"
#include	"language.h"
#include	"math.h"
#include	"memmgr.h"
#include "picture.h"
#include	"resource.h"
#include	"string.h"
#include	"text.h"
#include	"trig.h"
#include	"window.h"

static void	near SaveBackground(RWindow *);

#define	TITLEBAR	10		/* lines in title bar */

uint	winTitleBackColor = BRDCOLOR;
uint	winTitleTextColor = vWHITE;


global void InitWindow()	/* init window manager port and window list */
{
	wmgrPort = (RGrafPort *) NeedPtr(sizeof(RGrafPort));
	ROpenPort(wmgrPort);
	RSetPort(wmgrPort);
	RSetOrigin(0, picRect.top);
	wmgrPort->portRect.bottom = 200 - picRect.top;
	MoveRect(&rThePort->portRect, 0, 0);

/* init the window list */
	InitList(&windowList);
	AddToFront(&windowList, Pseudo(wmgrPort));
}


global void EndWindow()		/* Clean up window system preparatory to exit	*/
{
	DisposePtr(wmgrPort);
}



global RWindow * RGetWmgrPort()
{
	return((RWindow *) wmgrPort);
}



global void RBeginUpdate(uWind)	/* set up for a window update */
RWindow *uWind;
{
	RWindow *wind;
	RGrafPort *oldPort;

	RGetPort(&oldPort);
	RSetPort((RGrafPort *) RGetWmgrPort());

	wind = (RWindow *) Native(LastNode(&windowList));

	while (uWind != wind){
		SaveBackground(wind);
		wind = (RWindow *) Native(PrevNode(Pseudo(wind)));
	}

	RSetPort(oldPort);

}


global void REndUpdate(uWind)	/* end update phase of RWindow Manager */
RWindow *uWind;
{
	RGrafPort *oldPort;
	ObjID		uWindObj;

	RGetPort(&oldPort);
	RSetPort((RGrafPort *) RGetWmgrPort());

	uWindObj = Pseudo(uWind);
	while (uWindObj != LastNode(&windowList)){
		uWindObj = NextNode(uWindObj);
		uWind = (RWindow *) Native(uWindObj);
		SaveBackground(uWind);
	}				

	/* Re-Animate bits of the entire port */
	/* ReAnimate(&rThePort->portRect);	  */

	RSetPort(oldPort);
}


static void near SaveBackground(wind)
RWindow	*wind;
{
	Handle uBits;

	if (wind->mapSet && wind->visible) {
		uBits = SaveBits((RRect *) &wind->saveRect, VMAP);
		RestoreBits(wind->vUnderBits);
		wind->vUnderBits = uBits;

		if (wind->mapSet & PMAP) {
			uBits = SaveBits((RRect *) &wind->saveRect, PMAP);
			RestoreBits(wind->pUnderBits);
			wind->pUnderBits = uBits;
		}
	}
}


global bool RFrontWindow(wind)	/* return true if this is front window */
RWindow *wind;
{
	return(wind == (RWindow *) Native(LastNode(&windowList)));
}



global void RSelectWindow(wind)	/* put this window at end of list */
RWindow *wind;
{
	RWindow	*uWind;
	ObjID		windObj;

	RSetPort((RGrafPort *) wind);
	windObj = Pseudo(wind);
	if (windObj != LastNode(&windowList)){
		uWind = (RWindow *) Native(PrevNode(windObj));
		RBeginUpdate(uWind);
		MoveToEnd(&windowList, windObj);
		REndUpdate(uWind);
	}
	RSetPort((RGrafPort *) wind);
}



global RWindow * RNewWindow(fr, uFr, title, type, pri, vis)	/* Open and return pointer to */
register	RRect	*fr, *uFr;
strptr	title;
word		type, pri, vis;
{
	register	RRect		*wr;
	word		mapSet = VMAP;
	RRect		frame;
	RWindow	*wind;
	int		top, left;

/* allocate a structure for the window and add it to front of list */
	wind = (RWindow *) RNewPtr(sizeof(RWindow));
	if (wind == NULL) 
      {
		RAlert(E_OPEN_WINDOW);
		return NULL;
		}
	ClearPtr(wind);
	AddToEnd(&windowList, Pseudo(wind));

	/* initialize it the port structure */
 	ROpenPort((RGrafPort *) wind);
	RCopyRect(fr, &frame);
	RCopyRect(fr, &wind->port.portRect);
	if (uFr)
		RCopyRect(uFr, &wind->saveRect);
	wind->wType = type;
	wind->vUnderBits = 0;
	wind->pUnderBits = 0;
	wind->title = title;
	wind->visible = 0;

	/* determine mapSet we will save */
	if (!(type & NOSAVE)){
		if (pri != -1){
			mapSet |= PMAP;
		}
		wind->mapSet = mapSet;
	} else {
		wind->mapSet = 0;
	}
	
	/* get a copy of the title */
	if (title && (type & TITLED)){
		wind->title = (strptr) RNewPtr(1 + strlen(title));
		if (wind->title == NULL)
         {
			DisposePtr(wind);
			RAlert(E_OPEN_WINDOW);
			return NULL;
			}
		strcpy(wind->title, title);
	} else {
		wind->title = 0;
	}

	/* size the frame according to type */
	RCopyRect(fr, &frame);

	/* Adjust frame unless this is a custom window */
	if (wind->wType != CUSTOMWIND){
		if (!(type & NOBORDER)){
			RInsetRect(&frame, -1, 0);
			if (type & TITLED){
				frame.top -= TITLEBAR;
				++frame.bottom;
			} else {
				--frame.top;
			}
		/* add room for drop shadow */
			++frame.right;
			++frame.bottom;
		}
	}
	
	RCopyRect(&frame, &wind->frame);

	/* Try to get the window onto the screen, if possible.  These are ordered
	 * so that for a window too large to fit, the bottom and left of the window
	 * will be on the screen.
	 */
	fr = &wind->frame;
	wr = &wmgrPort->portRect;
	top = fr->top;
	left = fr->left;
	if (fr->top < wr->top)
		MoveRect(fr, fr->left, wr->top);
	if (fr->bottom > wr->bottom)
		MoveRect(fr, fr->left, fr->top - fr->bottom + wr->bottom);
	if (fr->right > wr->right)
		MoveRect(fr, fr->left - fr->right + wr->right, fr->top);
	if (fr->left < wr->left)
		MoveRect(fr, wr->left, fr->top);
	wr = &wind->port.portRect;
	MoveRect(wr, wr->left + fr->left - left, wr->top + fr->top - top);


	if (!uFr)
		RCopyRect(fr, &wind->saveRect);

	/* if vis arg is true we draw the window */
	if (vis)
		RDrawWindow(wind);

	/* make this window current port and massage portRect into content */
	RSetPort((RGrafPort *) wind);
	RSetOrigin(rThePort->portRect.left, rThePort->portRect.top + wmgrPort->origin.v);
	MoveRect(&rThePort->portRect, 0, 0);
	return(wind);
}


global void RDrawWindow(wind)	/* draw this window and set its visible flag */
RWindow *wind;
{
	RRect r;
	RGrafPort *oldPort;
	word oldPen;
	
	if (!wind->visible){
		wind->visible = 1;
	
	
	/* draw the window in master port according to type */
		RGetPort(&oldPort);
		RSetPort(wmgrPort);

		/* all drawing is done in window manager colors */
		PenColor(BRDCOLOR);

	/* save bits under frame (global coords) */
	/* and do priority fill if needed */
		if (!(wind->wType & NOSAVE)) {
		 /*	wind->vUnderBits = SaveBits(&wind->frame, VMAP);*/
				wind->vUnderBits = SaveBits(&wind->saveRect, VMAP);

			if (wind->mapSet & PMAP) {
				wind->pUnderBits = SaveBits(&wind->saveRect, PMAP);
				if (!(wind->wType & CUSTOMWIND))
					/* VCOLOR is ignored */
					RFillRect(&wind->saveRect, PMAP, 0, 15);
			}
		}


		/* A custom window is draw by the user */
		if (wind->wType != CUSTOMWIND){
			/* if nothing else we fill the entire frame */

			RCopyRect(&wind->frame, &r);

			/* now we work on the frame */
			if (!(wind->wType & NOBORDER)){
			/* drop shadow */
				--r.right;
				--r.bottom;
				ROffsetRect(&r, 1, 1);
				RFrameRect(&r);
				ROffsetRect(&r, -1, -1);
				RFrameRect(&r);

			/* are we titled */
				if (wind->wType & TITLED){
					r.bottom = r.top + TITLEBAR;
					RFrameRect(&r);
					RInsetRect(&r, 1, 1);
					RFillRect(&r, VMAP, winTitleBackColor);
					oldPen = rThePort->fgColor;
					PenColor(winTitleTextColor);
					if (wind->title)
						RTextBox(wind->title, 1, &r, TEJUSTCENTER, 0);
					PenColor(oldPen);

				/* return &r to where we were */
					RCopyRect(&wind->frame, &r);
					r.top += TITLEBAR - 1;
					--r.right;
					--r.bottom;
				}
				RInsetRect(&r, 1, 1);
			}

			/* fill content region (r) in background color of the windows port */
			if (!(wind->wType & NOSAVE)){
				RFillRect(&r, VMAP, wind->port.bkColor);
			}

		/* show what we have wrought */
			ShowBits(&wind->frame, VMAP);
		}
		RSetPort(oldPort);
	}
}

global void RDisposeWindow(wind, eraseOnly) /* dispose window and all allocated storage */
RWindow *wind;
bool eraseOnly;
{

	RSetPort(wmgrPort);
	RestoreBits(wind->vUnderBits);	/* frees handle */
	RestoreBits(wind->pUnderBits);	/* frees handle */
	(eraseOnly != 0)? ShowBits(&wind->saveRect, VMAP) : ReAnimate(&wind->saveRect);
	DeleteNode(&windowList, Pseudo(wind));
	RSelectWindow((RWindow *) Native(LastNode(&windowList)));
	if (wind->title)
		DisposePtr(wind->title);
	DisposePtr((memptr) wind);
	
}


global void RFrameRect(r)	/* draw a frame around this rect */
RRect *r;
{
	RRect rt;

	rt.top = r->top;
	rt.bottom = r->bottom;
	rt.left = rt.right = r->left;
	++rt.right;
	RPaintRect(&rt);

	rt.left = rt.right = r->right - 1;
	++rt.right;
	RPaintRect(&rt);

	rt.left = r->left;
	rt.top = rt.bottom = r->top;
	++rt.bottom;
	RPaintRect(&rt);

	rt.top = rt.bottom = r->bottom - 1;
	++rt.bottom;
	RPaintRect(&rt);
}


global void CenterRect(r)	/* center passed rectangle in wmgrPort */
RRect *r;
{
	word	ch, cv;
	RGrafPort *oldPort;

	RGetPort(&oldPort);
	RSetPort((RGrafPort *) RGetWmgrPort());
/* make zero origin for rectangle */
	ROffsetRect(r, -(r->left), -(r->top));

/* now offset it */
	ch = rThePort->portRect.right - rThePort->portRect.left;
	cv = rThePort->portRect.bottom - rThePort->portRect.top;
	ROffsetRect(r, (ch - r->right) / 2, (cv - r->bottom) / 2);
	RSetPort(oldPort);
}



global void RSetRect(rc, l, t, r, b)	/* make a rectangle from these values */
RRect *rc;
word l, r, t, b;
{

	rc->left = l;
	rc->right = r;
	rc->top = t;
	rc->bottom = b;
}


global bool REmptyRect(r)	/* return true if this is an empty rectangle */
RRect *r;
{
	return((r->right <= r->left) || (r->bottom <= r->top));
}


global void RCopyRect(src, dst)
RRect *src, *dst;
{
	*dst = *src;
/*
	*dst++ = *src++;
	*dst++ = *src++;
	*dst++ = *src++;
	*dst++ = *src++;
*/
}



global void RUnionRect(r1, r2, ru)	/* set ru to a rectangle encompassing both */
RRect *r1, *r2, *ru;
{

	if (r1->top < r2->top)
		ru->top = r1->top;
	else
		ru->top = r2->top;
	if (r1->left < r2->left)
		ru->left = r1->left;
	else
		ru->left = r2->left;

	if (r1->bottom > r2->bottom)
		ru->bottom = r1->bottom;
	else
		ru->bottom = r2->bottom;

	if (r1->right > r2->right)
		ru->right = r1->right;
	else
		ru->right = r2->right;
}




global bool RPtInRect(p, r)	/* true if point is in rect */
RPoint *p;
RRect *r;
{
	if (p->v < r->bottom && p->v >= r->top
	 	&& p->h < r->right && p->h >= r->left){
		return(1);
	} else {
		return(0);
	}
}




global int RPtToAngle(sp, dp)		/* return pseudo angle 0-359 */
RPoint *sp, *dp;
{
   return(ATan(dp->v,sp->h,sp->v,dp->h));
}

