/* MOTION
 * Kernel procedures for dealing with Actor motion and cycling.
 */


#include	"motion.h"
#include "selector.h"
#include	"start.h"
#include	"types.h"
#include	"pmachine.h"
#include	"grtypes.h"
#include	"cels.h"
#include "view.h"
#include	"kernel.h"
#include	"resource.h"
#include	"animate.h"
#include	"math.h"
#include	"string.h"
#include	"memmgr.h"
#include	"window.h"
#include	"dialog.h"
#include	"object.h"
#include	"stdlib.h"
#include "vm_defs.h"
#include	"errmsg.h"


#ifdef	LINT_ARGS

	static void	near DirLoop(Obj *, int);

#else

	static void	near DirLoop();

#endif

global void KBaseSetter(args)
word		*args;
{
	Obj	*actor;
	int		theY;
	word		scale;
  	aView far *viewPtr;
	Handle	view;


	actor = (Obj *) Native(arg(1));

	scale = GetProperty(actor, s_vm_signal);
	view = ResLoad(RES_VIEW, IndexedProp(actor, actView));
  	viewPtr = (aView far *) *view;
	

	if(!(scale & VM_SCALABLE) || !(viewPtr->vFlags & NOCOMPRESS))
		GetCelRect(view, 
			IndexedProp(actor, actLoop),
			IndexedProp(actor, actCel),
			IndexedProp(actor, actX),
			IndexedProp(actor, actY),
			IndexedProp(actor, actZ),
			(RRect *) IndexedPropAddr(actor, actBR));
	else
		{
			RCopyRect( (RRect *) IndexedPropAddr(actor, actNS), (RRect *) IndexedPropAddr(actor,actBR));
		}

	theY = 1 + IndexedProp(actor, actY);
	IndexedProp(actor, actBRBottom) = theY;
	IndexedProp(actor, actBR) = theY - IndexedProp(actor, actYStep);
}

global void KDirLoop(args)
word		*args;
{
	DirLoop((Obj *) Native(arg(1)), arg(2));
}





static void near DirLoop(actor, angle)
Obj	*actor;
int	angle;
/* This code correlates the desired direction to the proper loop.
 */
{
	int		loop, nLoops;

	/* Only set loop if loop is not fixed.
	 */
	if (!(fixedLoop & IndexedProp(actor, actSignal))) {
		nLoops = GetNumLoops(ResLoad(RES_VIEW, IndexedProp(actor, actView)));

		/* Set the loop for the actor based on how many loops it has.
		 */
		if (angle > 315 || angle < 45)
			loop = (nLoops >= 4)? 3 : -1;
		else if (angle > 135 && angle < 225)
			loop = (nLoops >= 4)? 2 : -1;
		else if ((angle < 180))
			loop = 0;
		else
			loop = 1;

		/* If the loop is not 'same' (-1), set it.
		 */
		if (loop != -1)
			IndexedProp(actor, actLoop) = loop;
		}
}





global void KInitBresen(args)
word		*args;
/* Initialize internal state of a motion class for a modified Bresenham line
 * algorithm (see 'bresen.doc' for derivation).
 */
{
	Obj	*motion, *client;
	int	DX, DY, toX, toY, dx, dy, incr, i1, i2, xAxis, di;
	int	tdx, tdy, watchDog;
	int	skipFactor;

	motion = (Obj *) Native(arg(1));
	client = (Obj *) Native(IndexedProp(motion, motClient));
	skipFactor = (argCount >= 2) ? arg(2) : 1;

	toX = IndexedProp(motion, motX);
	toY = IndexedProp(motion, motY);

	tdx = IndexedProp(client, actXStep) * skipFactor;
	tdy = IndexedProp(client, actYStep) * skipFactor;

	if (tdx > tdy)
		watchDog = tdx;
	else
		watchDog = tdy;

	watchDog *= 2;

	/* Get distances to be moved.
	 */
	DX = toX - IndexedProp(client, actX);
	DY = toY - IndexedProp(client, actY);

	/* Compute basic step sizes.
	 */
	forever {
		dx = tdx;
		dy = tdy;

		if (abs(DX) >= abs(DY)) {
			/* Then motion will be along the x-axis.
			 */
			xAxis = TRUE;
			if (DX < 0)
				dx = -dx;
			dy = (DX)? dx*DY/DX : 0;
			}
		else {
			/* Our major motion is along the y-axis.
			 */
			xAxis = FALSE;
			if (DY < 0)
				dy = -dy;
			dx = (DY)? dy*DX/DY : 0;
			}
	
		/* Compute increments and decision variable.
		 */
		i1 = (xAxis)? 2*(dx*DY - dy*DX) : 2*(dy*DX - dx*DY);
		incr = 1;
		if ((xAxis && DY < 0) || (!xAxis && DX < 0)) {
			i1 = -i1;
			incr = -1;
			}
	
		i2 = i1 - 2*((xAxis)? DX : DY);
		di = i1 - ((xAxis)? DX : DY);
	
		if ((xAxis && DX < 0) || (!xAxis && DY < 0)) {
			i1 = -i1;
			i2 = -i2;
			di = -di;
			}


		/* limit x step to avoid over stepping Y step size */
		if(xAxis && tdx > tdy){
			if (tdx && abs(dy + incr) > tdy){
				if(!(--watchDog)){
					RAlert(E_BRESEN);
					exit(1);
				}
				--tdx;
				continue;
			}
		}
		/* DON'T modify X
		 else {
			if (tdy && abs(dx + incr) > tdx){
				--tdy;
				continue;
			}
		}
		*/

		break;


	
	}
	/* Set the various variables we've computed.
	 */
	IndexedProp(motion, motDX) = dx;
	IndexedProp(motion, motDY) = dy;
	IndexedProp(motion, motI1) = i1;
	IndexedProp(motion, motI2) = i2;
	IndexedProp(motion, motDI) = di;
	IndexedProp(motion, motIncr) = incr;
	IndexedProp(motion, motXAxis) = xAxis;
}




void
KDoBresen(args)
word		*args;
/* Move an actor one step. */
{
	Obj		*motion, *client;
	int		x, y, toX, toY, i1, i2, di, si1, si2, sdi;
	int		xAxis, dx, dy, incr;
	memptr	aniState[500];

	motion = (Obj *) Native(arg(1));
	client = (Obj *) Native(IndexedProp(motion, motClient));
	acc = 0;

	IndexedProp(client, actSignal) = IndexedProp(client, actSignal)&(~ blocked);

	/* Get properties in variables for speed and convenience. */
	x = IndexedProp(client, actX);
	y = IndexedProp(client, actY);
	toX = IndexedProp(motion, motX);
	toY = IndexedProp(motion, motY);
	xAxis = IndexedProp(motion, motXAxis);
	dx = IndexedProp(motion, motDX);
	dy = IndexedProp(motion, motDY);
	incr = IndexedProp(motion, motIncr);
	si1 = i1 = IndexedProp(motion, motI1);
	si2 = i2 = IndexedProp(motion, motI2);
	sdi = di = IndexedProp(motion, motDI);

	IndexedProp(motion, motXLast) = x; 
	IndexedProp(motion, motYLast) = y; 

	/* Save the current animation state before moving the client. */
	memcpy((memptr) aniState, (memptr) client, 2 * ((Obj *) client)->size);

	if ((xAxis && (abs(toX - x) <= abs(dx))) || (!xAxis && (abs(toY - y) <= abs(dy)))) {
		/* We're within a step size of the destination -- set
		* client's x & y to it.
		*/
		x = toX;
		y = toY;
	}else {
		/* Move one step.
		*/
		x += dx;
		y += dy;
		if (di < 0)
			di += i1;
		else {
			di += i2;
			if (xAxis)
				y += incr;
			else
				x += incr;
		}
	}

	/* Update client's properties.
	*/
	IndexedProp(client, actX) = x;
	IndexedProp(client, actY) = y;
	/* Check position validity for this cel.
  	*/
	if (acc = InvokeMethod((Obj *) client, s_cantBeHere, 0)) {
		/* Client can't be here -- restore the original state and
  		* mark the client as blocked.
  		*/
		memcpy((memptr) client, (memptr) aniState, 2 * ((Obj *) client)->size);
		i1 = si1;
		i2 = si2;
		di = sdi;
		
		IndexedProp(client, actSignal) = blocked | IndexedProp(client, actSignal);
	}
	IndexedProp(motion, motI1) = i1;
	IndexedProp(motion, motI2) = i2;
	IndexedProp(motion, motDI) = di;
	if (x == toX && y == toY)
		InvokeMethod((Obj *) motion, s_moveDone, 0);
}


global void KDoAvoider()
{
}

/*
global void KDoAvoider(args)
word		*args;
{
	Obj		*avoider, *client, *motion, *looper;
	int		startDir, curDir, x, y, cx, cy, dx, dy, mx, my, heading;
	RPoint 	sp, dp;
	int     skipFactor;

	// Get the client and see if he's supposed to be moving.
	avoider = (Obj *) Native(arg(1));
	client = (Obj *) Native(IndexedProp(avoider, avClient));
	motion = (Obj *) Native(IndexedProp(client, actMover));
	skipFactor = (argCount >= 2) ? arg(2) : 1;

	if (!motion) {
		acc = -1;
		return;
		}

	// Do the motion.
	InvokeMethod(motion, s_doit, 0);

	// If the client no longer has a mover, he got where he wanted to be
	// and we're done.
	motion = (Obj *) Native(IndexedProp(client, actMover));
	if (!motion) {
		acc = -1;
		return;
		}

	x = IndexedProp(client, actX);
	y = IndexedProp(client, actY);
	mx = IndexedProp(motion, motX);
	my = IndexedProp(motion, motY);
	heading = IndexedProp(avoider, avHeading);

	// If the client is not blocked, reset the avoider heading and set
	// the loop.
	if (!InvokeMethod(client, s_isBlocked, 0)) {
		if (heading != -1) {
			heading = -1;
			sp.h = x;
			sp.v = y;
			dp.h = mx;
			dp.v = my;
			curDir = RPtToAngle(&sp, &dp);
			if (looper = (Obj *) Native(IndexedProp(client, actLooper)))
				InvokeMethod(looper, s_doit, 2, client, curDir);
			else
				DirLoop(client, curDir);
			}
		acc = -1;
		IndexedProp(avoider, avHeading) = heading;
		return;
		}

	if (heading == -1)
		heading = (RRandom() & 1)? 45 : -45;

	startDir = IndexedProp(client, actHeading);
	curDir = startDir = (startDir / 45) * 45;
	dx = IndexedProp(client, actXStep) * skipFactor;
	dy = IndexedProp(client, actYStep) * skipFactor;

	// Loop through directions, seeing if the object can move in one.
	forever {
		cx = x;
		cy = y;

		switch (curDir) {
			case 0:
				cy -= dy;
				break;
			case 45:
				cy -= dy;
				cx += dx;
				break;
			case 90:
				cx += dx;
				break;
			case 135:
				cy += dy;
				cx += dx;
				break;
			case 180:
				cy += dy;
				break;
			case 225:
				cy += dy;
				cx -= dx;
				break;
			case 270:
				cx -= dx;
				break;
			case 315:
				cy -= dy;
				cx -= dx;
				break;
			}

		// Set the client's position and determine whether he can be here.
		IndexedProp(client, actX) = cx;
		IndexedProp(client, actY) = cy;
		if (InvokeMethod(client, s_canBeHere, 0)) {
			acc = curDir;
			break;
			}

		// Client can't be here.  Try a different direction.
		curDir += heading;
		if (curDir >= 360)
			curDir -= 360;
		if (curDir < 0)
			curDir += 360;

		// If we're back to our starting direction, we can't move. Reset
		// the client's position to the original and return.
		if (curDir == startDir) {
			IndexedProp(client, actX) = x;
			IndexedProp(client, actY) = y;
			acc = -1;
			break;
			}
		}

		IndexedProp(avoider, avHeading) = heading;
}

*/



#define	labs(l)	(((l) < 0)? -(l) : (l))

global void KSetJump(args)
word	*args;
/* Compute the initial xStep for a motion of class Jump based on the
 * x and y differences of the start and end points and the force of
 * gravity.  This was downcoded from Script to use longs to avoid
 * overflow errors.
 */
{
	Obj	*theJump;
	long	DX, DY, denom;
	int	gy, n, xStep, yStep;

	theJump = (Obj *) Native(arg(1));
	DX = (long) arg(2);
	DY = (long) arg(3);
	gy = arg(4);
	
	/* For  most motion (increasing y or x motion comparable to or greater
	 * than y motion), we pick equal x & y velocities.  For motion which
	 * is mainly upward, we pick a y velocity which is n times that of x.
	 */
	n = 1;
	if (DX && (DY + labs(DX)) < 0)
		n = (int) ((2 * labs(DY)) / labs(DX));
		
	forever {
		denom = DY + n * labs(DX);
		if (labs(2 * denom) > labs(DX))
			break;
		++n;
	   }
	
	xStep = (denom)? sqrt(gy * DX * DX / (2 * denom)) : 0;
	
	/* Scale the y velocity, make sure that its sign is negative and that
	 * the x velocity is of the same sign as the x distance.
	 */
	yStep = n * xStep;
	if (yStep > 0)
		yStep = -yStep;
	if (DX < 0)
		xStep = -xStep;

	/* If we're supposed to move up and the y velocity is 0, recompute
	 * y based on no x movement.
	 */
	if (DY < 0 && yStep == 0)
		yStep = -1 - sqrt(-(2 * gy * DY));

	/* Set the jump properties.
	 */
	IndexedProp(theJump, jmpXStep) = xStep;
	IndexedProp(theJump, jmpYStep) = yStep;
}

