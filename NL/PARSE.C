/***************************************************************************

	Module     : PARSE
	
	Subprogram : NLP (Natural Language Parser Interface)
	
	Program    : SCI (Script Interpreter for Sierra On-line games)
	
	Author/Date: Pablo Ghenis for Sierra On-line, July 1988
	
	Platform   : Portable C, any 16+ bit micro with 512k+ RAM
	
	Purpose    : 
	             One of the two top-level modules in SCI Parser, provides the 
	             function newParse for the kernel to call (the other function 
	             required by the kernel is newSaid)
	             
	             Implements a Prolog-style backtracking parser driven by an 
	             external grammar which is loaded as a resource.
	             
	             nlsolve is the recursive engine for the backtracking 
	             machinery.
	            
***************************************************************************/



#ifndef	DLL_H
#include	"dll.h"
#endif

#ifndef	SCI_H
#include	"sci.h"
#endif

#ifndef	PARSE_H
#include	"parse.h"
#endif

#ifndef	STRING_H
#include	"string.h"
#endif

#ifndef	SELECTOR_H
#include "selector.h"
#endif

#ifndef	VOCAB_H
#include "vocab.h"
#endif

#ifndef	QUERY_H
#include "query.h"
#endif

#ifndef	SEMA_H
#include "sema.h"
#endif

#ifndef	PRINT_H
#include "print.h"
#endif

#ifndef	LISTS_H
#include "lists.h"
#endif

#ifndef	SYMS_H
#include "syms.h"
#endif

#ifndef	RESOURCE_H
#include	"resource.h"
#endif

#ifndef	OBJECT_H
#include	"object.h"
#endif



bool		NLDEBUG = FALSE;
bool		PoolDebug = FALSE;
ID			nl_event = NULL;
uint		MinStackLeft = 32000; /* bogus large positive number > 8k */
int		MAX_NODES_USED = 0;
bool		NL_cantSay = FALSE;
bool		PARSING_USER = FALSE;
nodeptr	NL_USER_PARSE_TREE = NULL;
nodeptr	PARSE_NODEPOOL = NULL;
ID			ParseGrammarID = 0;
ID			ParsePoolID = 0;



static uint			lastDobjVal=4096; /* A guaranteed non=match */
static NLCLAUSEDB NLCLAUSEARRAY = {0};


extern	memptr	stackBase;


#ifdef LINT_ARGS

void	_far _loadds	KParse(kArgs args);
bool		save_dobj(nodeptr);
nodeptr	find_by_slot(nodeptr,uint);
bool		nlsolvestart(NLTOK*,NLTAGS*);
bool		nlsolve(NLTOK*,NLTAGS*,NLQUERY,NLCLAUSEDB,nodeptr,nodeptr);
nodeptr	first_unresolved_node(nodeptr);
char*		get_token(char* , char*);
bool		isNumStr(char*);
uint		StackLeft(void);

#ifdef NOTDEF
nodeptr find_by_label(nodeptr,uint);
#endif

#else

void	_far _loadds	KParse();
bool		save_dobj();
nodeptr	find_by_slot();
bool		nlsolvestart();
bool		nlsolve();
nodeptr	first_unresolved_node();
char*		get_token();
bool		isNumStr();
uint		StackLeft();

#ifdef NOTDEF
nodeptr find_by_label();
#endif

#endif





/************************************

	Sierra Parser main module
	*************************


Unification grammar implementation with
backtracking at syntactic, semantic
and pragmatic level


************************************/




void
ParseInit()
/*  Do any initialization for the parser.
 */
{
   struct node anode;
	nodeptr	nodePool;

	ParseGrammarID = ResLoad(RES_VOCAB, 900);
	ResLock(RES_VOCAB, 900, TRUE);

	ParsePoolID = BlkNew(MAXPARSENODES*sizeof(anode));
	BlkUse(ParsePoolID);
	BlkPtrGet(ParsePoolID, nodePool);
   init_nodepool(nodePool, MAXPARSENODES);
}



void	_far _loadds
KParse (args)
kArgs	args;
/* Call parse to examine user input.
 */
{
	acc = Parse(arg(1), arg(2), arg(3));
}



bool 
Parse(lineID, evtID)
ID		lineID;
ID		evtID;
{
	strptr	inLine, inLinePtr;
	Object	*evt;
	int		j;
	char		p[LINESIZE];
	NLTOK		inTokens[MAXWORDS];
	NLTAGS	inTags[MAXWORDS];
	ID			wordID;
	bool		result=FALSE;

	inLine = StrPtrGet(lineID);
	BlkPtrGet(evtID, evt);
	BlkPtrGet(ParsePoolID, PARSE_NODEPOOL);
	NLCLAUSEARRAY = (NLCLAUSEDB) ResData(ParseGrammarID);

	/* Save event object for claiming in.
	 */
	nl_event = evtID;

	NL_cantSay = FALSE;
	
   /* Transform input string into an array of word numbers 
	 * and another of word descriptors
	 */
   inLinePtr = inLine;
   for (
			j = 0 ;
			(inLinePtr = get_token(inLinePtr, p) , *p && j <= MAXWORDS) ;
			++j
		 )
   {
      if (FindWordRoot(p))
         {
         inTokens[j] = wordVal;
         inTags[j]   = wordTags;
         
         if ((wordTags & PRON) && lastDobjVal) /* PRONOUN RECOGNITION */
            {
            inTokens[j] = lastDobjVal;
            inTags[j]   = NOUN;
            }
         }
      else if (isNumStr(p))
      	{
         inTokens[j] = 4093;		/* well-known "any-number" matcher */
         inTags[j]   = NOUN;
      	}
      else 
         {
         NL_cantSay=TRUE;
         result=FALSE;
			wordID = StrPtrDup(p);
			/* InvokeMethod(theGame, s_wordFail, 2, wordID, lineID); */
			(ift[INVOKEMETHOD].function)(theGame, s_wordFail, 2, wordID, lineID);
			BlkFree(wordID);
         break; /* out of this for-loop*/
         }   
   }
   inTokens[j] = NULL;
   inTags[j]   = NULL;
   
   if (j > 0)
      {
      if (!NL_cantSay)
         if (nlsolvestart(inTokens, inTags) == TRUE)
            result = TRUE;										/* SUCCESS POINT */
         else
            {
            NL_cantSay = TRUE;
            result = FALSE;			
				/* InvokeMethod(theGame, s_syntaxFail, 1, lineID); */
            (ift[INVOKEMETHOD].function)(theGame, s_syntaxFail, 1, lineID);
            }
      }
   else /* handle empty user input (j=0) */
      {
      NL_cantSay = TRUE;
      result = FALSE;
      }
    
	SetProperty(nl_event, s_claimed, NL_cantSay);
   return result;
}



/***************************************************************************

	internal FUNCTION DEFINITIONS

***************************************************************************/

/*
   User-parse enrty point before calling recursive solver
*/
bool 
nlsolvestart(intokens, intags)

	NLTOK* intokens;
	NLTAGS* intags;
{
   nodeptr rootnameptr, 
           treerootptr, 
           labelnameptr,
           treelabelptr,
           nodepool;
   bool    result;
   
   
   PARSING_USER=TRUE;

   nodepool = PARSE_NODEPOOL;


   poplistnode(treerootptr, nodepool);         /* start new tree */
   nodeassign(NL_USER_PARSE_TREE,treerootptr); /* secure the root */
   
   mark_and_sweep(0);                     /* just for consistent tests */
   
   poptokennode(rootnameptr,nodepool);         /* get node and link it */
   sethead(treerootptr,rootnameptr);
                  
   poplistnode(treelabelptr, nodepool);        /* get node and link it */
   settail(treerootptr,treelabelptr);
   
   poptokennode(labelnameptr,nodepool);        /* get node and link it */
   sethead(treelabelptr,labelnameptr);
   settail(treelabelptr,NULL);
   
   set_node_token(rootnameptr,NLROOT);
   set_node_token(labelnameptr,
                  clause_head(clausedb_head(NLCLAUSEARRAY)));
   
   

   result = nlsolve(intokens,
		            intags,
		            clause_expand(clausedb_head(NLCLAUSEARRAY)),
	               NLCLAUSEARRAY,
                  treerootptr,
                  nodepool);
                  
   
   PARSING_USER=FALSE;
   
   if (result) /* NEW STUFF */
      save_dobj(treerootptr);
   
   return result;
   }
   /* END nlsolvestart*/

/*
   Save the direct object's root noun to be used in next parse in place of 
   any pronoun
*/
bool
save_dobj(np) 

   nodeptr np;
   {
   for(np=find_by_slot(np,NLDOBJ);
       np;
       np=find_by_slot(np,NLROOT))
      {
      if (terminal_nodep(np))
         {
         lastDobjVal=termdatum(np);
         return TRUE;
         }
      }
   return FALSE;
   }
   /*end save_dobj*/
   
#ifdef notdef
nodeptr 
find_by_label(np,label) 

   nodeptr np;
   uint label;
   {
   nodeptr kids,kid;
   
   if (nodelabel(np)==label)
      return np;
   else
   if (terminal_nodep(np))
      return NULL;
   else
      for(kids=tail(np); kids;)
         {
         kids=tail(kids);
         kid=head(kids);
         if (nodelabel(kid)==label)
            return kid;
         }
         
      if (nodeslot(np)==NLROOT)
         np=find_by_slot(np,NLROOT);
         
      np=find_by_slot(np,NLROOT);
      
      return find_by_label(np,label);
   }
   /*end find_by_label*/
   
#endif /*ifdef notdef*/
   

nodeptr
find_by_slot(np, slot) 

   nodeptr np;
   uint slot;
   {
   nodeptr kids,kid;
   
   if (terminal_nodep(np))
      {
      if (nodeslot(np)==slot)
         return np;
      else
         return NULL;
      }
   else
     {
      kids=tail(np);
      while(kids=tail(kids))
         {
         kid=head(kids);
         if (nodeslot(kid)==slot)
            return kid;
         }
      return find_by_slot(find_by_slot(np,NLROOT),slot);
      }
   }
   /*end find_by_slot*/
   


bool
nlsolve(intokens,intags,query,clausedb,fulltree,nodepool)

   NLTOK*      intokens;
   NLTAGS*     intags;
   NLQUERY     query;
   NLCLAUSEDB  clausedb;
   nodeptr     fulltree;
   nodeptr     nodepool;
   
{
   
   NLTOK      newquery[NLQUERYSIZE]; /* allocate space for buffer query */
   NLQUERY    tmpquery,  
              head_expansion;  /* these are just pointers */
   NLCLAUSEDB tempdb;
   bool       solved,
              found_clause_group,
              give_up;
   nodeptr    node_to_augment,
	           tmp_node_to_augment;
   nodeptr    labelname,
              labelnode,
              slotname,
              slotnode,
              subtree,
              wordnode;
   nodeptr    savedtail,
              savedpool;

   uint StackNow;

   StackNow=StackLeft();
   
   if (StackNow<MinStackLeft)
      {
      MinStackLeft=StackNow;
      }
      
   if (StackNow<MINSOLVESTACK)
      {
      return FALSE;
      }

   if (empty_query(query))
      return (no_more_tokens(intokens) ? nlsema(fulltree) : FALSE);


   /* first unresolved node is defined as the first list node found
      in a depth-first search with a NULL tail. Not the fastest method
      but we're in a big hurry to get this code working. Probably not the 
      slowest for small trees like these...
   */
	node_to_augment = first_unresolved_node(fulltree);
         		   
	for( solved=FALSE
	     , found_clause_group=FALSE
	     , give_up=FALSE
	     , tempdb=clausedb
	     , querycpy(newquery,query);
	     
		  !solved && !EOGrammar(tempdb);
		  
        tempdb=clausedb_tail(tempdb))
   {
                    
      if (clause_match(clausedb_head(tempdb), query))
      {
         found_clause_group=TRUE;
         
         savedtail = tail(node_to_augment);
         savedpool = nodepool;
               
         if ( (word_clause(clausedb_head(tempdb))
               && nlunify(clause_speech_tag(clausedb_head(tempdb)),
	                       stream_head(intags) )
	           )
	           ||
	           word2_clause(clausedb_head(tempdb))
              ||
              (punct_clause(clausedb_head(tempdb))
               &&
               nlunify_punct(
                         clause_speech_tag(clausedb_head(tempdb)),
	                      stream_head(intokens) ) )
            )
            {
            popwordnode(wordnode,nodepool);
	         set_node_token(wordnode,stream_head(intokens));
            settail(node_to_augment, wordnode);
                          
               
            solved = nlsolve(stream_tail(intokens),
                             stream_tail(intags),
                             query_tail(query),
                             clausedb,
                             fulltree,
                             nodepool);
            }
         else
            {
            /* not a terminal resolution so EXPAND QUERY */
            
            head_expansion = clause_expand(clausedb_head(tempdb));
            
            /* proceed only if query is manageable in size
               otherwise we are probably pursuing a nonsensical
               hypothesis
            */
            
            if (append_queries( head_expansion,
                            query_tail(query),
                            newquery))
               {
            
               /* augment tree 
                  on subtree per subclause
               */
            
               for((tmpquery=head_expansion,
                    tmp_node_to_augment=node_to_augment);
                   query_head(tmpquery);
                   tmpquery=query_tail(tmpquery))
		   		   {
                  
                  poptokennode(labelname,nodepool);
                  set_node_token(labelname,query_head_label(tmpquery));
                  nlcons(labelnode, labelname, NULL,      nodepool);
#ifdef GC_H
                  nodelock(labelnode); /* to protect from GC */
#endif
                  poptokennode(slotname,nodepool);
                  set_node_token(slotname,query_head_slot(tmpquery));
                  nlcons(slotnode,  slotname,  labelnode, nodepool);
#ifdef GC_H
                  nodeunlock(labelnode);
#endif

                  nlcons(subtree,   slotnode,  NULL,      nodepool);
				   	
				   	/* finally link above to main tree (safe from GC) */
   
				   	settail(tmp_node_to_augment,subtree);
				   	
                  tmp_node_to_augment = subtree; /* for next pass */
                  }
            
   
               /* SOLVE RECURSIVELY */
   
               solved = nlsolve( intokens,
                                 intags,
                                 newquery,
                                 clausedb,
					   					fulltree,
						   				nodepool);

               }                

            } /*end if-else*/
         
         /* UNDO augmentation and unwind nodepool if failed */
            
         if (!solved)
            {
            settail(node_to_augment,savedtail);
            nodepool = savedpool;
            }
            
      } /*end if clausematch: keep trying*/
      
      else 
      
         give_up=found_clause_group;
         
   } /*end for*/

   return(solved);
   
   } 
   /* end nlsolve */


nodeptr
first_unresolved_node(root)
	nodeptr root;
	{
	nodeptr result=NULL;

	/* search depth-first for first listnode with a NULL tail */

	if (head(root) &&
		 getnodetype(head(root))==listtype)
	{
		result = first_unresolved_node(head(root));

		if (!result &&
			 tail(root) &&
			 getnodetype(tail(root))==listtype)
		{
			result = first_unresolved_node(tail(root));
		}
	}
	else
	{
		if (tail(root)==NULL)
		{
			result = root;
		}
		else if (getnodetype(tail(root))==listtype)
		{
			result = first_unresolved_node(tail(root));
		}
		else
		{
			result = NULL;
		}
	}

	return result;
} /* end first_unresolved_node */


char* get_token(s, token)

	char* s;
	char* token;
   {
   for(;*s && isspace(*s);)          /* skip initial white space */
      s++;
      
   while(*s && !isspace(*s))
      if (isalnum(*s))               /* accumulate alphanums */
         *token++=*s++;
      else
      if (ispunct(*s))               /* skip punctuation */
         s++;
      else
         break;
      
	*token='\0';                      /* always NULL-terminate token */
         
   for(;*s && isspace(*s);)          /* skip trailing white space */
      s++;
      
   return s;                         /* return remainder of input string */
   }
   

bool isNumStr(s)
char* s;
	{
	for (;*s;s++)
		if(!isdigit(*s)) return FALSE;
		
	return TRUE;
	}



uint
StackLeft()
{
	ubyte	i;

	return &i - stackBase;
}


/***************************************************************************

	END OF CODE FILE

***************************************************************************/




