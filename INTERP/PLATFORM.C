
#include	"kernel.h"
#include "platform.h"
#include "pmachine.h"

typedef enum 
   {
	Macintosh = 0,
	Dos,
	DosWindows,
	Amiga,
	WhatAmI
   } PlatformType;

/*
typedef enum 
   {
	Windows = 0,
   EGA
   CD
   } PlatformCall;
*/

global void KPlatform(args)
argList;
{
	acc = 0;

	if ( argCount < 2 )
		return;
	
	switch ( (PlatformType) arg(1) ) 
      {
		case Dos:
/*
			switch ( (PlatformCall) arg(2) ) 
            {
				case Windows:
					break;
				case EGA:
					break;
				case CD:
					break;
			   }
			break;
*/
		
		case Macintosh:
		case DosWindows:
		case Amiga: 
			break;	

		/*	Return the platform type that we are */

		case WhatAmI:
			acc = (word) Dos; 
			break;
	   }
}

