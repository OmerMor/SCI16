/***************************************************************************

	Module     : QUERY
	
	Subprogram : NLP (Natural Language Parser Interface)
	
	Program    : SCI (Script Interpreter for Sierra On-line games)
	
	Author/Date: Pablo Ghenis for Sierra On-line, July 1988
	
	Platform   : Portable C, any 16+ bit micro with 512k+ RAM
	
	Purpose    :
	             Support for main bactracking data structures
	

***************************************************************************/


#ifndef	DLL_H
#include	"dll.h"
#endif

#ifndef	QUERY_H
#include "query.h"
#endif

#ifndef	SAID_H
#include "said.h"
#endif



bool append_queries( q1, q2, newtokens)
               
	NLQUERY q1;
   NLQUERY q2; 
   NLQUERY newtokens;
   {
	int pos=0;

	newtokens[0] = NULL;	/* initialize just in case */

	/* queries as lists are NULL-terminated arrays */

	if (q1)
	   while (*q1)
	      newtokens[pos++]=*(q1++);
	   
	if (q2)
	   while (*q2)
		{
		   if (pos<NLQUERYSIZE-1)
			   newtokens[pos++]=*(q2++);
		   else 
		   {
			   /*
			   sprintf(message,"QOF %d\n",
			          pos); 
			   Alert(message);
			   */
			   return FALSE;
			}
		}
   newtokens[pos]=NULL;
   return (TRUE);
}

