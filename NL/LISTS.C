/***************************************************************************

	Module     : LISTS
	
	Subprogram : NLP (Natural Language Parser Interface)
	
	Program    : SCI (Script Interpreter for Sierra On-line games)
	
	Author/Date: Pablo Ghenis for Sierra On-line, July 1988
	
	Platform   : Portable C, any 16+ bit micro with 512k+ RAM
	
	Purpose    : 
	             Provide singly-linked nodes, lists and trees.
	             Provide node management/garbage collection
	
	

***************************************************************************/


#ifndef	LISTS_H
#include "lists.h"
#endif

#ifndef	DLL_H
#include	"dll.h"
#endif


#ifdef GC_H

#include "gc.c"

#else /* the rest of this file (kludge for testing phase) */


int LISTDEBUG=0;



void init_nodepool(nodepool, poolsize)

	nodeptr nodepool;
	uint poolsize;
	{
   uint i;

   for(i=1; i<poolsize; i++)
      setnodetype(nodepool++,virgintype);
   setnodetype(nodepool,endofpool);
}



nodeptr assoc(l, key)

	nodeptr l;
	NLTOK key;
{
   nodeptr result=NULL;
   
   for(;consp(l);l=tail(l))
   {
      if (consp(head(l))
          && tokenp(head(head(l)))
          && get_node_token(head(head(l)))==key)
      {
         result=head(l);
         break;
      }
   }
   return result;
}

#endif

