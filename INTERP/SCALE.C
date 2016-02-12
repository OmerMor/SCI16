/* * * * *
* VM_ZDRAW.C
* COPYRIGHT 1989 BY DYNAMIX, INC
*
* Contains vm_scrnload().
*
* MODIFICATION HISTORY
* NAME            DATE     WHAT
*
* P Lukaszuk      02/xx/91 Created
*
* M Peters        07/29/92 Changed vm_size routine to accept the x and y 
*                          offset in a cell and scale them with the scale factor 
*
* * * * */

#include "selector.h"

#include "vm_defs.h"
#include "view.h"
#include "types.h"
#include "grtypes.h"
#include "cels.h"
#include	"graph.h"
#include "object.h"
#include	"sciglobs.h"
#include	"errmsg.h"
#include	"pmachine.h"

struct line_incs
{
   long     acc;           // high word steps
   long     inc;           // fractional and whole components
};

#define  LINE_START(li)   (*((short *)(&((li).acc))+1))

// Put ending short val into high word of inc field.
#define  LINE_END(li)     (*((short *)(&((li).inc))+1))

// Retrieve the current val (high short of acc field).
#define  LINE_GET(li)    (*((short *)(&((li).acc))+1))

// Step the line in either direction.
#define  LINE_NEXT(li)   ((li).acc+=(li).inc)
#define  LINE_PREV(li)   ((li).acc-=(li).inc)

// Step the line as in LINE_NEXT and LINE_PREV, but also 'return' the
// new short val.
#define  LINE_GET_NEXT(li)   ((short)(LINE_NEXT(li)>>16))
#define  LINE_GET_PREV(li)   ((short)(LINE_PREV(li)>>16))

// Access low/high short of a long in 8088 
#define  LOW_SHORT(L)     (*((short *)(&L) + 0))
#define  HIGH_SHORT(L)    (*((short *)(&L) + 1))


unsigned short vm_deltaxs[ MAXWIDTH ], vm_deltays[ MAXHEIGHT ];
short far get_line_incs ( struct line_incs * , short );

/*#pragma  check_stack (off)*/
/*#pragma alloc_text (all, vm_size, get_line_incs )*/

/* Routine to obtain slope of our line */

short far get_line_incs ( struct line_incs * li, short steps )
{
   char isneg = 0;

   if ( steps <= 0 )
   {
      li->inc = 0;
      return ( LOW_SHORT(li->acc) = 0 );
   }

   LOW_SHORT(li->acc) = 0;
   LOW_SHORT(li->inc) = 0;
   li->inc -= li->acc;
   li->inc /= steps;

   // Flip back at end of rout.
   // 
   if ( li->inc < 0 )
   {
      li->inc = - li->inc;
      isneg = 1;
   }

   // If high word of increment is zero, and the fractional part is less
   // than 0x8000, then initialize fractional part of accumulator long
   // to be 0x8000.  This puts us half way into the first segment of lines
   // which rise very slowly.
   if ( ! ( li->inc & 0xffff8000 ) )
      LOW_SHORT(li->acc) = 0x8000;
   else
      LOW_SHORT(li->acc) = LOW_SHORT(li->inc);

   if ( isneg )
      li->inc = - li->inc;

   return 1;
}

/* Perform scaling by filling in tables that tell which pixels to draw when
	actually drawing scaled cell into video ram  */

void far vm_size(word oldW, word oldH, word oldXoff, word oldYoff,
                 word dxsize, word dysize ,word * newdxsize, word * newdysize,
                 word * newXoff, word * newYoff)
{
   short xx, yy  ;
	
	short dlx, dly;
   struct line_incs my_line;

	*newXoff = (oldXoff * dxsize) / VM_SCALE_BASE;  // must divide here not shift
	*newYoff = (oldYoff * dysize) / VM_SCALE_BASE;  // these values may be negative
	dxsize = ((uword)oldW * dxsize) >> VM_SCALE_SHIFT;
	dysize = ((uword)oldH * dysize) >> VM_SCALE_SHIFT;

	dlx = min( dxsize, MAXWIDTH  );
   dly = min( dysize, MAXHEIGHT );

/* Check for negative sizes */

	if(dlx < 0) dlx = 0;
	if(dly < 0) dly = 0;

	*newdxsize = dlx;
	*newdysize = dly;



/* If mirror is in effect, simply process in reverse direction */

   if( mirrored )
   {
      LINE_START(my_line) = oldW - 1;
      LINE_END(my_line) = 0;
   }
   else
   {
      LINE_START(my_line) = 0;
      LINE_END(my_line) =  oldW - 1;
   }
   get_line_incs ( & my_line, dxsize-1 );

/* Fill in table for x pixel offsets */   

   for( xx = 0; xx < dlx; xx++ )
   {
      vm_deltaxs[ xx ] = LINE_GET(my_line);
      LINE_NEXT(my_line);
   }
   vm_deltaxs[ xx ] ++;

   LINE_START(my_line) = 0;
   LINE_END(my_line) = oldH - 1;
   get_line_incs ( & my_line, dysize - 1 );

/* Fill in table for y pixel offsets */
	
   for( yy = 0; yy < dly; yy++ )
   {
      vm_deltays[yy] = LINE_GET(my_line);
      LINE_NEXT(my_line);
   }

}

/* Get new scale factors for auto scaling based upon vanishing point */


void  far GetNewScaleFactors(Obj  *vId, int height, word * newScaleX, word * newScaleY)
{
	int  bottom, pixsHigh ,newPixsHigh, vanY, yPos, vanHeight,vanYDist;
	Obj  *theRoom;
	

	yPos = IndexedProp(vId, actY);
	bottom = rThePort->portRect.bottom;

	pixsHigh = (GetProperty(vId, s_maxScale) * height) >> VM_SCALE_SHIFT ;
	
	theRoom = (Obj *) Native( globalVar[g_curRoom]);
	vanY = GetProperty(theRoom, s_vanishingY);

	vanHeight = bottom - vanY ;
	vanYDist = yPos - vanY ;
	if(vanYDist <= 0) vanYDist = 1;

	if( (vanHeight == 0) || (height == 0))
		Panic(E_SCALEFACTOR);

	newPixsHigh = ( pixsHigh * vanYDist) / vanHeight ;

	*newScaleY = (VM_SCALE_BASE * newPixsHigh) / height ;
	*newScaleX = *newScaleY ;
}
