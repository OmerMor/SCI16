/***************************************************************************

	Module     : PRINT
	
	Subprogram : NLP (Natural Language Parser Interface)
	
	Program    : SCI (Script Interpreter for Sierra On-line games)
	
	Author/Date: Pablo Ghenis for Sierra On-line, July 1988
	
	Platform   : Portable C, any 16+ bit micro with 512k+ RAM
	
	Purpose    : Output routines to display various structures used
	             in NLP, such as clauses, queries and lists
	             
	             This module is used mostly for debugging purposes. It will
                not fit in the (haha) remaining code space anyways. Since
  	             NLDEBUG is useless for debugging the parser, I recommend
  	             leaving NLDEBUG unlinked when using these routines. To leave 
  	             them out simply #define OMIT_PRINT in the header file and 
  	             recompile.	

***************************************************************************/



#ifndef	TYPES_H
#include "types.h"
#endif

#ifndef	DLL_H
#include	"dll.h"
#endif

#ifndef	LISTS_H
#include "lists.h"
#endif

#ifndef	PRINT_H
#include "print.h"
#endif

#ifndef	PARSE_H
#include "parse.h"
#endif

#ifndef	SAID_H
#include "said.h"
#endif

#ifndef	SYMS_H
#include "syms.h"
#endif

#ifndef	QUERY_H
#include "query.h"
#endif

#ifndef	RESOURCE_H
#include	"resource.h"
#endif




#define AlertSize messagesize


char*   message = 0;          /* string pointer, use for printing
                             messages. To save heap, allocate stack 
                             for it in top-level functions
                             */

#ifndef OMIT_PRINT /* rest of file */


static int NL_indent_step=3;
static int NL_indent=0;


#ifdef LINT_ARGS

   static void nodeprint(strptr, nodeptr, int);
   static void listprint(strptr, nodeptr, int);
   static void tokenprint(strptr, nodeptr, int);
   static void tagprint(strptr, nodeptr, int);
   static void wordprint(strptr, nodeptr, int);

   static void queryprint1(strptr, NLQUERY, int);
   static void queryprint(strptr, NLQUERY, int);

   static void clauseprint1(strptr, NLCLAUSE, int);
   static void clauseprint(strptr, NLCLAUSE, int);

   static void streamprint1(strptr, NLTOK*, int);
   static void streamprint(strptr, NLTOK*, int);

#else

   static void nodeprint();
   static void listprint();
   static void tokenprint();
   static void tagprint();
   static void wordprint();

   static void queryprint1();
   static void queryprint();

   static void clauseprint1();
   static void clauseprint();

   static void streamprint1();
   static void streamprint();

#endif /* #ifdef-else LINT_ARGS */




bool 
nodeAlert(title, np)

	strptr title;
	nodeptr np;
	{
   char s[AlertSize];
   
   strncpy(s,title,AlertSize);
   nodeprint(s,np,AlertSize);
   return Alert(s);
}

bool 
nodeAlert2(title, np, np2)

	strptr title;
	nodeptr np;
	nodeptr np2;
	{
   char s[AlertSize];
   
   strncpy(s,title,AlertSize);
   nodeprint(s,np,AlertSize);
   nodeprint(s,np2,AlertSize);
   return Alert(s);
   /*
   return nodeAlert(title,np) && nodeAlert("cont:",np2);
   */
}

bool 
queryAlert(title, Q)

	strptr title;
	NLQUERY Q;
	{
   char s[AlertSize];
   
   strncpy(s,title,AlertSize);
   queryprint(s,Q,AlertSize);
   return Alert(s);
}

bool 
clauseAlert(title, C)

	strptr title;
	NLCLAUSE C;
	{
   char s[AlertSize];
   
   strncpy(s,title,AlertSize);
   clauseprint(s,C,AlertSize);
   return Alert(s);
}

bool 
streamAlert(title, aStream)

	strptr title;
	NLTOK* aStream;
{
   char s[AlertSize];
   
   strncpy(s,title,AlertSize);
   streamprint(s,aStream,AlertSize);
   return Alert(s);
}


void 
report_nodepools()
{
   if (PoolDebug || NLDEBUG)
   {
      Alert("EOPool!");
      report_nodepool_use("PNodes= ",
                       PARSE_NODEPOOL,&MAX_NODES_USED);
      report_nodepool_use("SNodes= ",
                       Said_NODEPOOL,&MAX_SAIDNODES_USED);
   }
   
}

void 
report_nodepool_use(s,nodepool,MAX_USAGE_ADDRESS)
	char*   s;
	nodeptr nodepool;
	int*    MAX_USAGE_ADDRESS;
{
   int i;
   
   for(i=0;
       getnodetype(nodepool)!=virgintype
       && getnodetype(nodepool)!=endofpool;
       nodepool++)
      i++;
      
   if (i>*MAX_USAGE_ADDRESS)
   {
      *MAX_USAGE_ADDRESS=i;
      
      if (PoolDebug)
      {
         ift[SPRINTF].function(message,"%s %d",s,*MAX_USAGE_ADDRESS);
         Alert(message);    
      }
   }
}



/*

void
ParserProfile()
   {
   char s[300];
   char l[80];
   ulong LasttotalTicks;
   
   LasttotalTicks=LastParseTicks+LastSaidTicks+LastMatchTicks;
   ParseTicks+=LastParseTicks;
   SaidTicks+=LastSaidTicks;
   MatchTicks+=LastMatchTicks;
   totalTicks+=LasttotalTicks;
   
   *s='\0';

   if (ParseNumber>0)
      {
   sprintf(l,"calls:  P=%d S=%d/%d S/P=%d\n",
           ParseNumber,LastSaids,SaidNumber,SaidNumber/ParseNumber);
   strbcat(s,l,300);
   
   sprintf(l,"        P=%d%% S=%d%%\n",
           (100*ParseNumber)/(SaidNumber+ParseNumber),
           (100*SaidNumber)/(SaidNumber+ParseNumber));
   strbcat(s,l,300);
   
   sprintf(l,"1/10 s: P=%U S=%U M=%U T=%U\n",
           LastParseTicks/6,LastSaidTicks/6,LastMatchTicks/6,
           LasttotalTicks/6);
   strbcat(s,l,300);
   
   sprintf(l,"        P=%U%% S=%U%% M=%U%%\n\n",
           (100*LastParseTicks)/LasttotalTicks,
           (100*LastSaidTicks)/LasttotalTicks,
           (100*LastMatchTicks)/LasttotalTicks);
   strbcat(s,l,300);
   
   sprintf(l,"avrg:   P=%U S=%U M=%U T=%U\n",
           ParseTicks/(6*ParseNumber),
           SaidTicks/(6*ParseNumber),
           MatchTicks/(6*ParseNumber),
           totalTicks/(6*ParseNumber));
   strbcat(s,l,300);
   
   sprintf(l,"        P=%U%% S=%U%% M=%U%%",
           (100*ParseTicks)/totalTicks,
           (100*SaidTicks)/totalTicks,
           (100*MatchTicks)/totalTicks);
   strbcat(s,l,300);
   
   Alert(s);
   
      }
   
   LastParseTicks=0;
   LastSaidTicks=0;
   LastMatchTicks=0;
   LastSaids=0;
   }
*/
   /*END ParserProfile*/
   
/***************************************************************************

	internal FUNCTION DEFINITIONS

***************************************************************************/

void 
nodeprint(s,np,l)

	strptr s;
	nodeptr np;
	int l;
	{
   int i;

   if (!np)
      strbcat(s,"NIL",l);
   else
   switch( getnodetype(np) ) {
      case
      virgintype:  /*
                  */
                  strbcat(s,"<VN>",l);
                  break;
      case
      emptytype:  /*
                  */
                  strbcat(s,"<EN>",l);
                  break;
      case
      tagtype:    tagprint(s,np,l);
						break;
		case
		wordtype:   wordprint(s,np,l);
						break;
      case
      tokentype:  tokenprint(s,np,l);
						break;
      case
      listtype:
                  /*
                  */
                  strbcat(s,"\r",l);
                  for(i=0;i<=NL_indent;i++) strbcat(s," ",l);
                  NL_indent=NL_indent+NL_indent_step;
                  strbcat(s,"(",l);
                  listprint(s,np,l);
                  strbcat(s,")",l);
                  NL_indent=NL_indent-NL_indent_step;
                  break;
      case
      endofpool:  /*
                  */
                  strbcat(s,"<EOP>",l); 
                  break;

      default:    
                  ift[SPRINTF].function(message,"<TYPE%d?>",getnodetype(np));
                  strbcat(s,message,l);
   }
}


void 
listprint(s, np, l)

	strptr s;
	nodeptr np;
	int l;
{
   if (np)
   {
      if (np->type==listtype)
      {
  	         nodeprint( s, head(np), l );

            if ( !nullp( tail(np) ) )
               strbcat(s," ",l);

				listprint(s,tail(np),l);
      }
		else
		{
			/*
			*/
			strbcat(s," . ",l);
			nodeprint(s,np,l);
		}
	}
}

void 
tokenprint(s, np, l)

	strptr s;
	nodeptr np;
	int l;
{
   char theSym[NLSYMSIZE];
/*
*/
	if (getnodetype(np)==tokentype) 
	{
      tok_to_sym(np->data.token, NLSYMTAB, theSym);
	   strbcat(s,theSym,l) ;
	}
	else
		strbcat(s,"<NONTOK>",l);
}

void 
tagprint(s, np, l)

	strptr s;
	nodeptr np;
	int l;
{
/*
*/
   char stmp[20];

	if (getnodetype(np)==wordtype)
	{
	   ift[SPRINTF].function(stmp,"w%x",np->data.tags);
		strbcat(s,stmp,l);
	}
	else
		strbcat(s,"<NONTAG>",l);
}

void 
wordprint(s, np, l)

	strptr s;
	nodeptr np;
	int l;
{
/*
*/ 
   char stmp[20];

	if (getnodetype(np)==wordtype)
	{
	   if (np->data.token >= 0x1000)
	   {
	      tok_to_sym(np->data.token,NLSYMTAB,stmp);
	      /*
	      sprintf(stmp,"x%x",np->data.token);
	      */
	   }
	   else
	      ift[SPRINTF].function(stmp,"w%d",np->data.token);
	      
		strbcat(s,stmp,l);
	}
	else
		strbcat(s,"<NONWRD>",l);
}





void 
queryprint1(s, q, l)

	strptr s;
	NLQUERY q;
	int l;
{
	NLTOK atok1,atok2;
	char stmp1[NLSYMSIZE]
	     ,stmp2[NLSYMSIZE];
	char stmp[2*NLSYMSIZE+5];
/*
*/
	for(;query_head(q);q=query_tail(q))
	{
		atok1=query_head_slot(q);
		atok2=query_head_label(q);
		
		tok_to_sym(atok1, NLSYMTAB, stmp1);
		tok_to_sym(atok2, NLSYMTAB, stmp2);
		
		ift[SPRINTF].function(stmp," (%s %s) ", stmp1, stmp2);
		
	   strbcat(s,stmp,l);
	}
}

void 
queryprint(s, q, l)

	strptr s;
	NLQUERY q;
	int l;
{
	strbcat(s,"(",l);
	queryprint1(s,q,l);
	strbcat(s,")\r",l);
/*
*/
}

void 
clauseprint1(s, c, l)

	strptr s;
	NLCLAUSE c;
	int l;
{
   char stmp[NLSYMSIZE];
   
	tok_to_sym(clause_head(c),NLSYMTAB,stmp);
	strbcat(s,stmp,l);
	strbcat(s," ",l);
	queryprint1(s,clause_tail(c),l);
/*
*/
}

void 
clauseprint(s, c, l)

	strptr s;
	NLCLAUSE c;
	int l;
{
	strbcat(s,"( ",l);
	clauseprint1(s,c,l);
	strbcat(s," )\r",l);
/*
*/
}

void 
streamprint1(s, astream, l)

	strptr s;
	NLTOK* astream;
	int l;
{
   char tmp[NLSYMSIZE];
   
	while(stream_head(astream))
	{
	   itoa(stream_head(astream),tmp,10);
		strbcat(s," ",l);
	   strbcat(s,tmp,l);
		strbcat(s," ",l);
		astream=stream_tail(astream);
	}
/*
*/
}

void 
streamprint(s,astream,l)

	strptr s;
	NLTOK* astream;
	int l;
{
	strbcat(s,"(",l);
	streamprint1(s,astream,l);
	strbcat(s,")\r",l);
/*
*/
}


#endif /* ifndef OMIT_PRINT */
