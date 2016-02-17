/***************************************************************************

	Module     : SEMA
	
	Subprogram : NLP (Natural Language Parser Interface)
	
	Program    : SCI (Script Interpreter for Sierra On-line games)
	
	Author/Date: Pablo Ghenis for Sierra On-line, July 1988
	
	Platform   : Portable C, any 16+ bit micro with 512k+ RAM
	
	Purpose    : 
	             SEMAntic filter, third in the 
	             lexical/syntactic/semantic/pragmatic checking sequence.
	
	

***************************************************************************/


#ifndef	DLL_H
#include	"dll.h"
#endif

#ifndef	SEMA_H
#include "sema.h"
#endif

#ifndef	PRAGMA_H
#include "pragma.h"
#endif



#ifdef LINT_ARGS

	static bool nlsema_check(nodeptr);

#else

	static bool nlsema_check();

#endif /* #ifdef-else LINT_ARGS */




bool nlsema(np)

	nodeptr np;
{
   if (nlsema_check(np))
      return nlpragma(np);
   else
      return FALSE;

}



bool nlsema_check(np)

	nodeptr np;
{
   if (consp(np))     /* bogus temporary check */
      return TRUE;
   else
      {
      Alert("sema err: not a tree");
      return FALSE;
      }
}

