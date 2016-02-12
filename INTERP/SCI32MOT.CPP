// motion.cpp
//		kernel procedures for dealing with Actor motion and cycling.

#include <string.h>

#include "sol.hpp"

#include "errmsg.hpp"
#include "graph.hpp"
#include "kernel.hpp"
#include "math.hpp"
#include "object.hpp"
#include "pmachine.hpp"
#include "rect.hpp"
#include "resource.hpp"
#include "sci.hpp"
#include "selector.hpp"

static void	DirLoop(ObjectID, int);

void
KBaseSetter(kArgs)
{
#if 0
	ObjectID	actor = arg(1);
	int		scale = actor.GetProperty(s_vm_signal);
	MemID		viewID = resMgr->Get(VIEW, actor[actView]);
	View*		view = (View*) *viewID;

	if (!(scale & VM_SCALABLE) || !(view->vFlags & NOCOMPRESS))
		GetCelRect(viewID,
			actor[actLoop],
			actor[actCel],
			actor[actX],
			actor[actY],
			actor[actZ],
			(SOL_Rect*) &actor[actBR]);
	else
		*(SOL_Rect*) &actor[actBR] = *(SOL_Rect*) &actor[actNS];

	int y = actor[actY] + 1;
	actor[actBRBottom] = (Property) y;
	actor[actBR] = Property(y - actor[actYStep]);
#endif
}

void
KDirLoop(argList)
{
	DirLoop(arg(1), arg(2));
}

void
KInitBresen(argList)
{
	//	Initialize internal state of a motion class for a modified Bresenham line
	// algorithm (see 'bresen.doc' for derivation).

	ObjectID	motion = arg(1);
	ObjectID	client = motion[motClient];
	int		dx, dy, incr, i1, i2, xAxis, di;
	int		watchDog;
	int		skipFactor = argCount >= 2 ? arg(2) : 1;
	int		toX = motion[motX];
	int		toY = motion[motY];
	int		tdx = client[actXStep] * skipFactor;
	int		tdy = client[actYStep] * skipFactor;

	if (tdx > tdy)
		watchDog = tdx;
	else
		watchDog = tdy;

	watchDog *= 2;

	// Get distances to be moved.
	int DX = toX - client[actX];
	int DY = toY - client[actY];

	// Compute basic step sizes.
	while (1) {
		dx = tdx;
		dy = tdy;

		if (Abs(DX) >= Abs(DY)) {
			// Then motion will be along the x-axis.
			xAxis = True;
			if (DX < 0)
				dx = -dx;
			dy = DX ? dx * DY / DX : 0;

		} else {
			// Our major motion is along the y-axis.
			xAxis = False;
			if (DY < 0)
				dy = -dy;
			dx = DY ? dy * DX / DY : 0;
		}
	
		// Compute increments and decision variable.
		i1 = xAxis ? 2 * (dx * DY - dy * DX) : 2 * (dy * DX - dx * DY);
		incr = 1;
		if ((xAxis && DY < 0) || (!xAxis && DX < 0)) {
			i1 = -i1;
			incr = -1;
		}
	
		i2 = i1 - 2 * (xAxis ? DX : DY);
		di = i1 - (xAxis ? DX : DY);
	
		if ((xAxis && DX < 0) || (!xAxis && DY < 0)) {
			i1 = -i1;
			i2 = -i2;
			di = -di;
		}

		// limit x step to avoid over stepping Y step size
		if (xAxis && tdx > tdy) {
			if (tdx && Abs(dy + incr) > tdy){
				if (!(--watchDog))
					Panic(E_BRESEN);
				--tdx;
				continue;
			}
		}

		break;
	
	}
	// Set the various variables we've computed.
	motion[motDX] = (Property) dx;
	motion[motDY] = (Property) dy;
	motion[motI1] = (Property) i1;
	motion[motI2] = (Property) i2;
	motion[motDI] = (Property) di;
	motion[motIncr] = (Property) incr;
	motion[motXAxis] = (Property) xAxis;
}

void
KDoBresen(argList)
{
	// Move an actor one step

	ObjectID motion = arg(1);
	ObjectID client = motion[motClient];
	int		i1, i2, di, si1, si2, sdi;
	char		aniState[1000];

	sv.acc = 0;

	client[actSignal] = Property(client[actSignal] & ~blocked);

	// Get properties in variables for speed and convenience
	int x			= client[actX];
	int y			= client[actY];
	int toX		= motion[motX];
	int toY		= motion[motY];
	int xAxis	= motion[motXAxis];
	int dx		= motion[motDX];
	int dy		= motion[motDY];
	int incr		= motion[motIncr];
	si1 = i1		= motion[motI1];
	si2 = i2		= motion[motI2];
	sdi = di		= motion[motDI];

	motion[motXLast] = (Property) x;
	motion[motYLast] = (Property) y;

	// Save the current animation state before moving the client
	//	this will be replaced!
	memcpy(aniState, (Object*) client, client->size * sizeof(Property));

	if ((xAxis && (Abs(toX - x) <= Abs(dx))) ||
		 (!xAxis && (Abs(toY - y) <= Abs(dy)))) {
		/* We're within a step size of the destination -- set
			client's x & y to it.
		*/
		x = toX;
		y = toY;

	} else {
		// Move one step.
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

	// Update client's properties.
	client[actX] = (Property) x;
	client[actY] = (Property) y;

	// Check position validity for this cel.
	if (sv.acc = InvokeMethod(client, s_cantBeHere)) {
		/* Client can't be here -- restore the original state and
			mark the client as blocked.
		*/
		memcpy((Object*) client, aniState, client->size * sizeof(Property));
		i1 = si1;
		i2 = si2;
		di = sdi;

		client[actSignal] = Property(client[actSignal] | blocked);
	}
	motion[motI1] = (Property) i1;
	motion[motI2] = (Property) i2;
	motion[motDI] = (Property) di;
	if (x == toX && y == toY)
		InvokeMethod(motion, s_moveDone);
}

void
KSetJump(argList)
{
	/* Compute the initial xStep for a motion of class Jump based on the
	 * x and y differences of the start and end points and the force of
	 * gravity.  This was downcoded from Script to use longs to avoid
	 * overflow errors.
	 */

	long	denom;
	int	n, xStep, yStep;

	ObjectID theJump = arg(1);
	long DX = arg(2);
	long DY = arg(3);
	int gy = arg(4);
	
	/* For  most motion (increasing y or x motion comparable to or greater
	 * than y motion), we pick equal x & y velocities.  For motion which
	 * is mainly upward, we pick a y velocity which is n times that of x.
	 */
	n = 1;
	if (DX && (DY + Abs(DX)) < 0)
		n = (int) ((2 * Abs(DY)) / Abs(DX));
		
	while (1) {
		denom = DY + n * Abs(DX);
		if (Abs(2 * denom) > Abs(DX))
			break;
		++n;
	   }
	
	xStep = denom ? sqrt(gy * DX * DX / (2 * denom)) : 0;
	
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

	// Set the jump properties.
	theJump[jmpXStep] = (Property) xStep;
	theJump[jmpYStep] = (Property) yStep;
}

void
KCantBeHere(argList)
{
	// determine and return legality of actors position
	// This code checks base rect intersection

	ObjectID		him = arg(1);
	SOL_ListID	cast = arg(2);
	ObjectID		me;
	SOL_Rect		r((SOL_Rect&) him[actBR]);
	
	/* (s_illegalBits) are the bits that the object cannot be on.
		Anding this with the bits the object is on tells us which
		bits the object is on but shouldn't be on.  If this is zero,
		the position is valid.
	 */
	sv.acc = 0;
	// if I am hidden or ignoring actors my position is legal
	if (!(him[actSignal] & (ignrAct | HIDDEN))) {
		// default to no hits
		sv.acc = 0;
		// now the last thing we care about is our rectangles
		for (KNodeID node = cast->First(); !!node; node = node->Next()) {
			me = node->val;
			// Can't hit myself
			if (him == me)
				continue;

			// can't hit if I'm as below
			if (me[actSignal] & (NOUPDATE | ignrAct | HIDDEN))
				continue;

			// if our rectangles intersect we are done
			SOL_Rect*	chkR = (SOL_Rect*) &me[actBR];
			if (r.x1 >= chkR->x2 || r.x2 <= chkR->x1 ||
				 r.y1 >= chkR->y2 || r.y2 <= chkR->y1) {
				continue;
			} else {
				sv.acc = (Acc) me;
				break;
			}
		}
	}
}

static void
DirLoop(ObjectID actor, int angle)
{
	//	This code correlates the desired direction to the proper loop.

	// Only set loop if loop is not fixed.
	if (!(fixedLoop & actor[actSignal])) {
		int loop;
		int nLoops = GetNumLoops(resMgr->Get(VIEW, actor[actView]));

		// Set the loop for the actor based on how many loops it has.
		if (angle > 315 || angle < 45)
			loop = (nLoops >= 4)? 3 : -1;
		else if (angle > 135 && angle < 225)
			loop = (nLoops >= 4)? 2 : -1;
		else if ((angle < 180))
			loop = 0;
		else
			loop = 1;

		// If the loop is not 'same' (-1), set it.
		if (loop != -1)
			actor[actLoop] = (Property) loop;
	}
}
