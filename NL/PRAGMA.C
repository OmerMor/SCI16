/***************************************************************************

	Module     : PRAGMA
	
	Subprogram : NLP (Natural Language Parser Interface)
	
	Program    : SCI (Script Interpreter for Sierra On-line games)
	
	Author/Date: Pablo Ghenis for Sierra On-line, July 1988
	
	Platform   : Portable C, any 16+ bit micro with 512k+ RAM
	
	Purpose    :
	             Final level check in the
	             lexical/syntactic/semantic/pragmatic sequence
	
***************************************************************************/

/***************************************************************************

	#include HEADERS
	
	(to pick up external declarations for types,
	variables, functions, constants and macros)

***************************************************************************/


#ifndef	TYPES_H
#include "types.h"
#endif

#ifndef	DLL_H
#include	"dll.h"
#endif

#ifndef	PRAGMA_H
#include "pragma.h"
#endif

#ifndef	LISTS_H
#include "lists.h"
#endif

#ifndef	PRINT_H
#include "print.h"
#endif



bool nlpragma(np)

	nodeptr np;
   {
   if (consp(np))
      {
      return TRUE;
      } 
   else
      {
      Alert("err: Pragma(NIL)");
      return FALSE;
      }
   }


