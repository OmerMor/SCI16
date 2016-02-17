/***************************************************************************

	Module     : SAID
	
	Subprogram : NLP (Natural Language Parser Interface)
	
	Program    : SCI (Script Interpreter for Sierra On-line games)
	
	Author/Date: Pablo Ghenis for Sierra On-line, July 1988
	
	Platform   : Portable C, any 16 bit micro with 512k RAM
	
	Purpose    : Parse Said-spec tokens to generate tree
	             Compare Said-trees to user input trees
	
	

***************************************************************************/



#ifndef	DLL_H
#include	"dll.h"
#endif

#ifndef	BLOCK_H
#include	"block.h"
#endif

#ifndef	SAID_H
#include "said.h" 
#endif

#ifndef	SAIDF_H
#include "saidf.h"
#endif

#ifndef	PARSE_H
#include "parse.h"
#endif

#ifndef	LISTS_H
#include "lists.h"
#endif

#ifndef	SYMS_H
#include "syms.h"
#endif

#ifndef	PRINT_H
#include "print.h"
#endif

#ifndef	VOCAB_H
#include "vocab.h"
#endif

#ifndef	EVENT_H
#include	"event.h"
#endif

#ifndef	SELECTOR_H
#include	"selector.h"
#endif



#define NLNOWORD    4094
#define NLANYWORD   4095
#define MISMATCH   -1


#define StackNeeded(n)			if(StackLeft()<n)	\
								 			{Alert("out of said stack"); return FALSE;}

#define ORsubtree(spec)			(nodelabel(spec)==NLOR       \
										|| nodelabel(spec)==NLOR2)
#define OPTsubtree(spec)		(nodeslot(spec)==NLOPT)                    
#define ROOTsubtree(spec)		(nodeslot(spec)==NLROOT)                    
#define reducible_spec(np)		(tokenp(head(np))            \
                            	&& (ROOTsubtree(np)         \
                              	|| OPTsubtree(np))      \
                            	&& !terminal_nodep(np))
#define term_match(spec,data) (termdatum(spec)!=NLNOWORD             \
                               && (termdatum(spec)==termdatum(data)  \
                                   || termdatum(spec)==NLANYWORD     \
                                   || termdatum(data)==NLANYWORD))

                            
#define said_match(parstree,spectree) \
        (match_datatree_to_spectree(parstree,spectree)==TRUE)


bool		SaidDebug = TRUE;
bool		PARSING_SAID = FALSE;
int		MAX_SAIDNODES_USED = 0;
NLTOK*	intokens = NULL;
nodeptr	NLSFpool = NULL;
nodeptr	Said_NODEPOOL = NULL; 
ID			SaidPoolID = 0;


static bool    ORing  = FALSE;
static bool    OPTing = FALSE;

static nodeptr NL_SAID_TREE=NULL;



#ifdef LINT_ARGS

   static bool   NLSaid(NLTOK* spec);

   static void   NLSaid_Decompress(memptr *spec, NLTOK* newspec);

   static bool   NLSaidspecAlert(char*,NLTOK*);

   static int    match_data_to_speclist(nodeptr, nodeptr);
   static int    match_data_to_spectree(nodeptr, nodeptr);
   static int    match_datatree_to_spectree(nodeptr, nodeptr);

   static bool   unify_term_spec(nodeptr,nodeptr);
   static bool   unify_list_spec(nodeptr,nodeptr);

#else

   static bool   NLSaid();

   static void   NLSaid_Decompress();

   static bool   SaidspecAlert();
   static bool   NLSaidspecAlert();

   static int    match_data_to_speclist();
   static int    match_data_to_spectree();
   static int    match_datatree_to_spectree();

   static bool   unify_term_spec();
   static bool   unify_list_spec();

#endif /* #ifdef-else LINT_ARGS */




void
SaidInit()
{
   struct node anode;
	nodeptr	nodePool;
   
	SaidPoolID = BlkNew(MAXSAIDNODES*sizeof(anode));
	BlkUse(SaidPoolID);
	BlkPtrGet(SaidPoolID, nodePool);
   init_nodepool(nodePool, MAXSAIDNODES);
}



void	_far _loadds
KSaid(args)			/* return truth or falsity of "said spec" */
kArgs	args;
{
	acc = (arg(1))? Said(arg(1), nl_event) : FALSE;
}



bool	_far _loadds
Said(specID, eventID)
ID		specID;
ID		eventID;
/* 
  Provide same functional interface as before for compatibility
   will only make use of FIRST argument though
*/
{
	memptr	spec;
   NLTOK		nlspec[MaxSaidSize];    /*allocate room for decompressed spec*/
   bool		result;
	char		message_allocation[messagesize]; /* allocate message on stack */

   message=message_allocation;           /* give the storage to global */
	
	/* we should kick out if: */
	if (NL_cantSay = 	(!eventID)								/* no event */
		|| !specID													/* no said spec */
		||	GetProperty(eventID, s_claimed)					/* or event claimed */
		|| !(GetProperty(eventID, s_type) & saidEvent)	/* or not a saidEvent */
		)
      return FALSE;
      
   StackNeeded(300);
   
   PARSING_SAID=TRUE;
   NL_cantSay=TRUE;
   
	BlkPtrGet(specID, spec);
	BlkPtrGet(SaidPoolID, Said_NODEPOOL);
   NLSaid_Decompress(&spec, nlspec); /*decompress it*/

   result = NLSaid(nlspec);           /*check it out*/

   if (result!=TRUE)
      NL_cantSay=FALSE;
      
   PARSING_SAID=FALSE;
	SetProperty(eventID, s_claimed, NL_cantSay);
   
   return result;
}




static bool NLSaidspecAlert(title, spec)
	char* title;
	NLTOK *spec;
{
   char tmp[20],puncts[10];
   NLTOK tok;
   
   strncpy(puncts,",&/()[]#<>",10);
   strncpy(message,title,messagesize);
   
   for(; *spec!=NULL; spec++)
   {
	   if (*spec >= 0x1000)
         if (*spec >= 0xf000)
            { 
            tok=((*spec)>>8)-0x00f0;
            if (tok>=0 && tok<=9)
		         (ift[SPRINTF].function)(tmp," %c",puncts[tok]);
	         }
	      else
	         tok_to_sym(*spec,NLSYMTAB,tmp);
	   else
	      (ift[SPRINTF].function)(tmp," %d",*spec);
         
      strbcat(message,tmp,messagesize);
   }
   return Alert(message);
}



static void
NLSaid_Decompress(spec, newspec)
memptr	*spec;
NLTOK*	newspec;
{
   ubyte token;
   NLTOK newtok; /* an NLTOK is just a uword */
   
   token=*(*spec)++;  /* get token as in old Said code */

      if (hibiton(token))
      {
         if (EOspec(token))               /* METAEND = 1 byte */
         {
            *newspec = NULL;
            return;
         }
         else                             /* PUNCTUATION = 1 byte*/
         {
            newtok = ((NLTOK)token)<<8;
            *newspec++ = newtok ;
            NLSaid_Decompress(spec,newspec);
         }
      }
      else                                /* A REAL WORD = 2 bytes*/
      {
        newtok =  ((NLTOK)token)<<8;    /* flip word as in old Said code */
        newtok |= *(*spec)++;
        *newspec++ = newtok;
        
        NLSaid_Decompress(spec,newspec);
      }
}

static bool NLSaid(spec)
	NLTOK* spec;
{
   bool    ValidSpec,result;

   nodeptr rootnameptr, 
           treerootptr, 
           labelnameptr,
           treelabelptr;
   
   
   StackNeeded(300);
   
   NLSFpool = Said_NODEPOOL;

   poplistnode(treerootptr, NLSFpool);
   nodeassign(NL_SAID_TREE,treerootptr); /* secure the root */

   poptokennode(rootnameptr,NLSFpool);   /* get node and link it */
   set_node_token(rootnameptr,NLROOT);
   sethead(treerootptr,rootnameptr);
                  
   poplistnode(treelabelptr, NLSFpool);   /* get node and link it */
   settail(treerootptr,treelabelptr);

   poptokennode(labelnameptr,NLSFpool);   /* get node and link it */
   set_node_token(labelnameptr,NLS);
   sethead(treelabelptr,labelnameptr);

   settail(treelabelptr,NULL);
   
   
   intokens=spec;
   ValidSpec = NLSF_S(treelabelptr); /* treelabelptr is node_to_augment */

           
   if (ValidSpec)
      {
   
      /*
         deref root of parse tree just in case it got moved around
      */
#ifndef GC_H
		BlkPtrGet(ParsePoolID, NL_USER_PARSE_TREE);
#endif

#ifdef DEBUGGABLE      
      if (SaidDebug)
         SaidDebug=NLSaidspecAlert("spec",spec) 
                   && nodeAlert("spec:",NL_SAID_TREE)
                   && nodeAlert("input:",NL_USER_PARSE_TREE);
#endif

      result=(said_match(NL_USER_PARSE_TREE,NL_SAID_TREE)==TRUE);
      }  
   else
      {
      NLSaidspecAlert("Bad Said Spec\nhit return to continue\n",spec); 
      result=FALSE;
      }
   
   return result;
}

/*****************************************************************

   given two trees, one derived from user input and the other from 
   parsing a tokenized said-spec, determine whether they match
   
*****************************************************************/

/*

TREE MATCHING ALGORITHM: (slightly obsolete)

match data/speclist 
match data/speclist

   for each subtree in speclist
      find a match with data
      if OR-ing break on first match
      if not OR-ing break on first failure
      if OPT-ing break on first mismatch
      
match datalist/spectree

   if spec is not root
      find matching entry in datalist
      match entry to spec
   if still no result
      find root entry in datalist
      match entry to spec

match datatree/spectree

      if data label is not root or same as spec
         FALSE
      else
         if spec is term
            if data is term
               if datafields are the same
                  TRUE
               else
                  FALSE
            else
               unify spec term with reduced data list
         else
            unify reduced spec list with data

*/


static int match_datatree_to_spectree(data, spec)
	nodeptr data;
	nodeptr spec;
{
   int result;
   bool SavedORing,SavedOPTing;
   
   StackNeeded(300);
   
   if (nodeslot(spec)==NLMORE)
   {
      NL_cantSay=FALSE;
      return TRUE;
   }
   
   SavedORing = ORing;
   SavedOPTing = OPTing;
      
   ORing = ORsubtree(spec);
   OPTing = OPTsubtree(spec);
      
   if (	 !ROOTsubtree(data)
       && !ROOTsubtree(spec)
       && !OPTsubtree(spec)
       && nodeslot(data)!=nodeslot(spec))
       
      result = MISMATCH;
      
   else if (terminal_nodep(spec))
   
      if (terminal_nodep(data))
      
         result = term_match(spec,data)
                  ? TRUE 
                  : MISMATCH;
                  
      else if (ROOTsubtree(data) 
               || nodeslot(data)==nodeslot(spec))
      
         result = match_data_to_spectree(treetail(data),spec);
         
      else
      
         result=FALSE;
         
   else if (terminal_nodep(data))
   
      if (nodeslot(spec)!=NLROOT
          && nodeslot(spec)!=NLOPT
          && nodeslot(data)!=nodeslot(spec))
          
         result=FALSE;
         
      else
      
         result = match_data_to_speclist(data,treetail(spec));
         
   else if (reducible_spec(spec) || nodeslot(data)==nodeslot(spec))
   
      result = match_data_to_speclist(treetail(data),treetail(spec));
      
   else
   
      result = match_data_to_spectree(treetail(data),spec);
   
   if (OPTing && result==FALSE)
      result=TRUE;
         
   ORing = SavedORing;
   OPTing = SavedOPTing;

   return result;
}

static int match_data_to_spectree(data, spec)
	nodeptr data;
	nodeptr spec;
{
   int result, tmpresult;
   bool SavedORing,SavedOPTing;
   nodeptr subtree;
   
   StackNeeded(300);
   
   if (nodeslot(spec)==NLMORE)
   {
      NL_cantSay=FALSE;
      return TRUE;
   }
   
   SavedORing = ORing;
   SavedOPTing = OPTing;
   
   ORing = ORsubtree(spec);
   OPTing = OPTsubtree(spec);
   
   if (reducible_spec(spec))
   {
      result=match_data_to_speclist(data,treetail(spec));
   }
   else if (treelistp(data))
   {
      for(result=tmpresult=FALSE;
          consp(data) && result!=TRUE;
          data=tail(data))
      {
         subtree = head(data);
         if (nodeslot(subtree)==nodeslot(spec))
         {
            tmpresult = match_datatree_to_spectree(subtree,spec);
         }
         else
         if (nodeslot(subtree)==NLROOT /* && result != MISMATCH */) /* PG 9/11/88 */
         	
         {
            tmpresult  = match_datatree_to_spectree(subtree,spec);
         }
         
         if (result==MISMATCH)
            {
            if (tmpresult==TRUE)
               result=TRUE;
            }
         else
            result=tmpresult;   
      }
   }
   else
      result = match_datatree_to_spectree(data,spec);
   
   if (OPTing && result==FALSE)
      result=TRUE;
         
   ORing = SavedORing;
   OPTing = SavedOPTing;
      
   return result;
}

static int match_data_to_speclist(data, spec)
	nodeptr data;
	nodeptr spec;
{
   /*
   match a speclist to a data node or list of nodes
   ie. for each spec element find a math in the data
   maximize search space by reducing the spec first
   */
   int result;
   
   StackNeeded(300);
   
   for(result=TRUE;consp(spec);spec=tail(spec))
   {
      if (nodeslot(head(spec))==NLIGNR)
         continue;
         
      result=match_data_to_spectree(data,head(spec));
      
      if (ORing)
         if (result!=TRUE)
            continue;
         else
            break;
      else
      if (result!=TRUE)
         break;
      /*      
      if (OPTing && result==MISMATCH)
      {
         result=FALSE;
         break;
      }
      */
   }
   
   return result;
}

