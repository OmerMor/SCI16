//	mouse.c

#include "graph.h"
#include "mouse.h"

bool	mouseIsDebug = FALSE;
word	buttonState = 0;

word
RGetMouse(RPoint* pt)
{
	// Return interrupt level position in local coords of mouse in the point

	pt->v = mouseY - rThePort->origin.v;
	pt->h = mouseX - rThePort->origin.h;
	return buttonState;
}

word
CurMouse(RPoint* pt)
{
	// Return interrupt level position in global coords of mouse in the point

	pt->v = mouseY;
	pt->h = mouseX;
	return buttonState;
}
