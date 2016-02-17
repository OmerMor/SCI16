/***************************************************************************

	Module     : SAIDF
	
	Subprogram : NLP (Natural Language Parser Interface)
	
	Program    : SCI (Script Interpreter for Sierra On-line games)
	
	Author/Date: Pablo Ghenis for Sierra On-line, July 1988
	
	Platform   : Portable C, any 16+ bit micro with 512k+ RAM
	
	Purpose    :
	             Natural Language Said FAST
Fast version of Said-spec parser and tree builder
Hard-coded in C as a recursive-descent parser based on defunct NLSGRMR.txt
	

***************************************************************************/


#ifndef	TYPES_H
#include "types.h"
#endif

#ifndef	DLL_H
#include	"dll.h"
#endif

#ifndef	SAIDF_H
#include "saidf.h"
#endif

#ifndef	SAID_H
#include "said.h"
#endif

#ifndef	LISTS_H
#include "lists.h"
#endif

#ifndef	SYMS_H
#include "syms.h"
#endif

#ifndef	QUERY_H
#include "query.h"
#endif



#define NLSF_PHR(anode) NLSF_UNIT(anode)

#define StackNeeded(n) if(StackLeft()<n) return FALSE;


bool hadPhrase=TRUE;


#ifdef LINT_ARGS

bool augment_tree(nodeptr,NLTOK,NLTOK,nodeptr);

bool NLSF_SPEC(nodeptr);
bool NLSF_SPEC2(nodeptr);
bool NLSF_DOBJ(nodeptr);
bool NLSF_IOBJ(nodeptr);

bool NLSF_PHR2(nodeptr);
bool NLSF_UNIT(nodeptr);
bool NLSF_MOD(nodeptr);
bool NLSF_OR2(nodeptr);
bool NLSF_OR(nodeptr);
bool NLSF_ATOM(nodeptr);
bool NLSF_WORD(nodeptr);

bool NLSF_PUNCT(nodeptr,NLTOK);
bool NLSF_MUNCH(NLTOK);

#else

bool augment_tree();

bool NLSF_SPEC();
bool NLSF_SPEC2();
bool NLSF_DOBJ();
bool NLSF_IOBJ();

bool NLSF_PHR2();
bool NLSF_UNIT();
bool NLSF_MOD();
bool NLSF_OR2();
bool NLSF_OR();
bool NLSF_ATOM();
bool NLSF_WORD();

bool NLSF_PUNCT();
bool NLSF_MUNCH();

#endif /* #ifdef-else LINT_ARGS */




bool NLSF_S(node_to_augment)

	nodeptr node_to_augment;
   {
   NLTOK*  savedtokens;
   nodeptr savedtail,savedpool;
   bool    result;
   
   StackNeeded(300);
   nodelock(node_to_augment);
   
   savedtail = tail(node_to_augment);
   savedpool = NLSFpool;
   savedtokens = intokens;
   
   if (!(result=(NLSF_SPEC(node_to_augment) && no_more_tokens(intokens))))
      {
      settail(node_to_augment,savedtail);
      NLSFpool=savedpool;
      intokens = savedtokens;
      }
     
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_S*/
   




bool augment_tree(node_to_augment, SLOT_TOK, LABEL_TOK, KID)    
                  
      nodeptr node_to_augment;
      NLTOK SLOT_TOK;
      NLTOK LABEL_TOK;
      nodeptr KID;
      {                                                  
      nodeptr slotname,slotnode,labelname,labelnode,subtree;
   
      nodelock(KID); /* not linked to tree yet, so protect from GC */
      
      poptokennode(slotname,NLSFpool);   /*NLSFpool is global*/                
      set_node_token(slotname,SLOT_TOK);    
      nodelock(slotname);             
                                                         
      poptokennode(labelname,NLSFpool);                  
      set_node_token(labelname,LABEL_TOK);               
      nodelock(labelname);             
                                                         
                                                         
      if (consp(KID))
         {
         labelnode=KID;
         sethead(labelnode,labelname);
         }
      else
         {
         labelnode=(nodeptr)NULL;
         Alert("augment_tree err: non-consp KID");
         }
         
      nlcons(slotnode,  slotname,  labelnode,      NLSFpool); 
      nlcons(subtree,   slotnode,  (nodeptr)NULL,  NLSFpool);  

	   settail(node_to_augment,subtree);       

      nodeunlock(slotname);             
      nodeunlock(labelname);    
      
      nodeunlock(KID); /* no need for locks, now linked to main tree */         
	   
	   return TRUE;           
	   }


bool NLSF_SPEC(node_to_augment)

	nodeptr node_to_augment;
   {
   bool result;
   NLTOK*  savedtokens;
   nodeptr savedtail,savedpool;
   nodeptr nodeTA;
   
   StackNeeded(300);

   nodelock(node_to_augment);
   nodeTA=node_to_augment;
   
   savedtail = tail(node_to_augment);
   savedpool = NLSFpool;
   savedtokens = intokens;                                                   
                                                                             
   result=NLSF_SPEC2(nodeTA);
   
   if (!result)
      {
      settail(node_to_augment,savedtail);
      NLSFpool=savedpool;
      intokens = savedtokens;
      }
     
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_SPEC*/
   
bool NLSF_SPEC2(node_to_augment)

	nodeptr node_to_augment;
   {
/*
dw NLSPEC2,NLROOT,NLPHR,  NLROOT,NLDOBJ,  NLROOT,NLIOBJ, NULL,NULL,     NULL
dw NLSPEC2,NLROOT,NLPHR,  NLROOT,NLDOBJ,  NULL,NULL,     NULL,NULL,     NULL
dw NLSPEC2,NLROOT,NLPHR,  NULL,NULL,      NULL,NULL,     NULL,NULL,     NULL
dw NLSPEC2,NLROOT,NLDOBJ, NULL,NULL,      NULL,NULL,     NULL,NULL,     NULL
dw NLSPEC2,NLROOT,NLDOBJ, NLROOT,NLIOBJ,  NULL,NULL,     NULL,NULL,     NULL

dw NLSPEC2,NLIGNR,NLLP,   NLROOT,NLSPEC2, NLIGNR,NLRP,   NULL,NULL,     NULL
dw NLSPEC2,NLROOT,NLSPEC2,NLIGNR,NLCOMA,  NLROOT,NLSPEC2,NULL,NULL,     NULL
*/
   bool result=FALSE;
   NLTOK*  savedtokens;
   nodeptr savedtail,savedpool;
   nodeptr anode,nodeTA;
   
   StackNeeded(300);

   nodelock(node_to_augment);
   nodeTA=node_to_augment;
   
   savedtail = tail(node_to_augment);
   savedpool = NLSFpool;
   savedtokens = intokens;                                                   
   
   poplistnode(anode,NLSFpool);
   if (NLSF_PHR(anode)) 				/* all roots are equal, 
                           			 only some are more equal than others */
      {
      result=TRUE;
      augment_tree(nodeTA,NLROOT,NLPHR,anode);
      nodeTA=tail(nodeTA);
      }
         
   if (NLSF_DOBJ(nodeTA))				/* direct object */
      {
      result=TRUE;
      if (hadPhrase) 
      	nodeTA=tail(nodeTA);

   	if (NLSF_IOBJ(nodeTA))				/* maybe also indirect object */
  			if (hadPhrase) 
      		nodeTA=tail(nodeTA);
     	
      } 
      
      
   poplistnode(anode,NLSFpool);
   if (NLSF_PUNCT(anode,NLRNON))						/* runon may follow */
      augment_tree(nodeTA,NLMORE,NLRNON,anode);
	   
   if (!result)
      {
      settail(node_to_augment,savedtail);
      NLSFpool=savedpool;
      intokens = savedtokens;
      }
     
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_SPEC2*/
   
bool NLSF_DOBJ(node_to_augment)

	nodeptr node_to_augment;
   {
/*
dw NLDOBJ, NLDOBJ,NLPHR2, NULL,NULL,      NULL,NULL,     NULL,NULL,     NULL
dw NLDOBJ, NLIGNR,NLLB,   NLOPT,NLDOBJ,   NLIGNR,NLRB,   NULL,NULL,     NULL
*/
   bool result;
   NLTOK*  savedtokens;
   nodeptr savedtail,savedpool;
   nodeptr anode;
   
   StackNeeded(300);

   nodelock(node_to_augment);
   
   savedtail = tail(node_to_augment);
   savedpool = NLSFpool;
   savedtokens = intokens;                                                   
   hadPhrase=TRUE;
   
   poplistnode(anode,NLSFpool);
   if (result=NLSF_PHR2(anode))
      augment_tree(node_to_augment,NLDOBJ,NLPHR2,anode);
   else
   if (result=(NLSF_MUNCH(NLLB)
               && NLSF_DOBJ(anode)
               && NLSF_MUNCH(NLRB)))
      augment_tree(node_to_augment,NLOPT,NLDOBJ,anode);
   else
   if (result=(NLSF_MUNCH(NLSLSH)))
   	hadPhrase=FALSE;						/* a slash is an acceptable no-op */
   else
      {
      settail(node_to_augment,savedtail);
      NLSFpool=savedpool;
      intokens = savedtokens;
      hadPhrase=FALSE;
      }
     
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_DOBJ*/
   
bool NLSF_IOBJ(node_to_augment)

	nodeptr node_to_augment;
   {
/*
dw NLIOBJ, NLIOBJ,NLPHR2, NULL,NULL,      NULL,NULL,     NULL,NULL,     NULL
dw NLIOBJ, NLIGNR,NLLB,   NLOPT,NLIOBJ,   NLIGNR,NLRB,   NULL,NULL,     NULL
*/
   bool result;
   NLTOK*  savedtokens;
   nodeptr savedtail,savedpool;
   nodeptr anode;
   
   StackNeeded(300);

   nodelock(node_to_augment);
   
   savedtail = tail(node_to_augment);
   savedpool = NLSFpool;
   savedtokens = intokens;                                                   
   hadPhrase=TRUE;
   
   poplistnode(anode,NLSFpool);
   
   if (result=NLSF_PHR2(anode))
   
      augment_tree(node_to_augment,NLIOBJ,NLPHR2,anode);
      
   else
   if (result=(NLSF_MUNCH(NLLB)
               && NLSF_IOBJ(anode)
               && NLSF_MUNCH(NLRB)))
               
      augment_tree(node_to_augment,NLOPT,NLIOBJ,anode);
      
   else
   if (result=(NLSF_MUNCH(NLSLSH)))
  	 	hadPhrase=FALSE;						/* a slash is an acceptable no-op */
   else
      {
      settail(node_to_augment,savedtail);
      NLSFpool=savedpool;
      intokens = savedtokens;
      hadPhrase=FALSE;
      }
     
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_IOBJ*/
   
/********************************************
NLSF_PHR should be a macro equal to NLSF_UNIT
see top of file
********************************************/
   
bool NLSF_PHR2(node_to_augment)

	nodeptr node_to_augment;
   {
/*
dw NLPHR2, NLIGNR,NLSLSH, NLROOT,NLUNIT,  NULL,NULL,     NULL,NULL,     NULL
;;; dw NLPHR2, NLIGNR,NLSLSH, NULL,NULL,      NULL,NULL,     NULL,NULL,     NULL
*/
   bool result;
   NLTOK*  savedtokens;
   nodeptr savedtail,savedpool;
   
   StackNeeded(300);

   nodelock(node_to_augment);
   
   savedtail = tail(node_to_augment);
   savedpool = NLSFpool;
   savedtokens = intokens;                                                   
   
   result=NLSF_MUNCH(NLSLSH) && NLSF_UNIT(node_to_augment);
        
   if (!result)
      {
      settail(node_to_augment,savedtail);
      NLSFpool=savedpool;
      intokens = savedtokens;
      }
     
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_PHR2*/
   
bool NLSF_UNIT(node_to_augment)

	nodeptr node_to_augment;
   {
/*
dw NLUNIT,NLROOT,NLOR,  NLROOT,NLMOD,  NULL,NULL,     NULL,NULL,     NULL
dw NLUNIT,NLROOT,NLOR,  NULL,NULL,      NULL,NULL,     NULL,NULL,     NULL
dw NLUNIT,NLROOT,NLMOD, NULL,NULL,      NULL,NULL,     NULL,NULL,     NULL
*/
   bool result=FALSE;
   NLTOK*  savedtokens;
   nodeptr savedtail,savedpool;
   nodeptr anode,nodeTA;
   
   StackNeeded(300);

   nodelock(node_to_augment);
   nodeTA=node_to_augment;
   
   savedtail = tail(node_to_augment);
   savedpool = NLSFpool;
   savedtokens = intokens;                                                   
   
   
   /* an OR root requires an extra level because it is the only case where
      the LABEL (as opposed to the slot) matters to the matcher. This should 
      be changed since it is an unnecessary inconsistency
      */
   
   poplistnode(anode,NLSFpool);
   if (NLSF_OR(anode))
      {
      result=TRUE;
      augment_tree(nodeTA,NLROOT,NLOR,anode);
      nodeTA=tail(nodeTA);
      }
	   
   if (NLSF_MOD(nodeTA))
      {
      result=TRUE;
	   }
      
   if (!result)
      {
      settail(node_to_augment,savedtail);
      NLSFpool=savedpool;
      intokens = savedtokens;
      }
     
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_UNIT*/

   
bool NLSF_MOD(node_to_augment)

	nodeptr node_to_augment;
   {
/*
dw NLMOD, NLIGNR,NLLESS, NLMOD,NLOR,   NULL,NULL,     NULL,NULL,     NULL
dw NLMOD, NLIGNR,NLLESS, NLMOD,NLOR,   NLROOT,NLMOD,  NULL,NULL,     NULL
dw NLMOD, NLIGNR,NLLB,   NLOPT,NLMOD,  NLIGNR,NLRB,   NULL,NULL,     NULL
*/
   bool result;
   NLTOK*  savedtokens;
   nodeptr savedtail,savedpool;
   nodeptr anode,nodeTA;
   
   StackNeeded(300);

   nodelock(node_to_augment);
   nodeTA=node_to_augment;
   
   savedtail = tail(node_to_augment);
   savedpool = NLSFpool;
   savedtokens = intokens;                                                   
   
   poplistnode(anode,NLSFpool);
   if (result=(NLSF_MUNCH(NLLESS) && NLSF_OR(anode)))
      {
      augment_tree(nodeTA,NLMOD,NLOR,anode);
      nodeTA=tail(nodeTA);
      
      poplistnode(anode,NLSFpool);
      if (NLSF_MOD(anode))
         {
         augment_tree(nodeTA,NLROOT,NLMOD,anode);
         }
      }
   else
   if (result=(NLSF_MUNCH(NLLB)
               && NLSF_MOD(anode)
               && NLSF_MUNCH(NLRB)))
      augment_tree(nodeTA,NLOPT,NLMOD,anode);
   else
      {
      settail(node_to_augment,savedtail);
      NLSFpool=savedpool;
      intokens = savedtokens;
      }
     
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_MOD*/
   
bool NLSF_OR2(node_to_augment)

	nodeptr node_to_augment;
   {
/*
dw NLOR2,  NLIGNR,NLCOMA, NLROOT,NLOR,    NULL,NULL,     NULL,NULL,     NULL
*/
   bool result;
   NLTOK*  savedtokens;
   nodeptr savedtail,savedpool;
   
   StackNeeded(300);

   nodelock(node_to_augment);
   
   savedtail = tail(node_to_augment);
   savedpool = NLSFpool;
   savedtokens = intokens;                                                   
   
   if (!(result=(NLSF_MUNCH(NLCOMA) && NLSF_OR(node_to_augment))))
      {
      settail(node_to_augment,savedtail);
      NLSFpool=savedpool;
      intokens = savedtokens;
      }
     
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_OR2*/
   
bool NLSF_OR(node_to_augment)

	nodeptr node_to_augment;
   {
/*
dw NLOR,   NLROOT,NLATOM, NULL,NULL,      NULL,NULL,     NULL,NULL,     NULL
dw NLOR,   NLROOT,NLATOM, NLROOT,NLOR2,   NULL,NULL,     NULL,NULL,     NULL
*/
   bool result;
   NLTOK*  savedtokens;
   nodeptr savedtail,savedpool;
   nodeptr nodeTA;
   
   StackNeeded(300);

   nodelock(node_to_augment);
   nodeTA=node_to_augment;
   
   savedtail = tail(node_to_augment);
   savedpool = NLSFpool;
   savedtokens = intokens;                                                   
   
   if (result=NLSF_ATOM(nodeTA))
      {
      nodeTA=tail(nodeTA);
      
      NLSF_OR2(nodeTA);
      }
   else
      {
      settail(nodeTA,savedtail);
      NLSFpool=savedpool;
      intokens = savedtokens;
      }
     
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_OR*/
   
bool NLSF_ATOM(node_to_augment)

	nodeptr node_to_augment;
   {
/*
dw NLATOM, NLIGNR,NLLB,   NLOPT,NLUNIT,   NLIGNR,NLRB,   NULL,NULL,     NULL
dw NLATOM, NLIGNR,NLLP,   NLROOT,NLUNIT,  NLIGNR,NLRP,   NULL,NULL,     NULL
dw NLATOM, NLROOT,NLWORD, NULL,NULL,      NULL,NULL,     NULL,NULL,     NULL
*/
   bool    result=FALSE;
   NLTOK*  savedtokens;
   nodeptr savedtail,savedpool;
   nodeptr anode;
   
   StackNeeded(300);

   nodelock(node_to_augment);
   
   savedtail = tail(node_to_augment);
   savedpool = NLSFpool;
   savedtokens = intokens;
   
   poplistnode(anode,NLSFpool);
   if (NLSF_MUNCH(NLLB))
      {
      if (result=(NLSF_UNIT(anode) && NLSF_MUNCH(NLRB)))
         augment_tree(node_to_augment,NLOPT,NLUNIT,anode);   
      }
   else if (NLSF_MUNCH(NLLP))
      {
      if (result=NLSF_UNIT(anode) && NLSF_MUNCH(NLRP))
         augment_tree(node_to_augment,NLROOT,NLUNIT,anode);   
      }
   else
      {
      result=NLSF_WORD(anode);
      augment_tree(node_to_augment,NLROOT,NLWORD,anode); 
      }
   
   if (!result)
      {
      settail(node_to_augment,savedtail);
      NLSFpool=savedpool;
      intokens = savedtokens;
      }
     
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_ATOM*/
   
bool NLSF_WORD(node_to_augment)

	nodeptr node_to_augment;
   {
   nodeptr anode;
   bool    result=FALSE;
   
   nodelock(node_to_augment);
   
   if (result=(*intokens 
               && !(*intokens & 0x8000))) /*not a word but a punct mark*/
      {   
      popwordnode(anode,NLSFpool);

      set_node_token(anode,*intokens++);
      settail(node_to_augment,anode);
      }  
       
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_WORD*/
   
bool NLSF_PUNCT(node_to_augment, amark)

	nodeptr node_to_augment;
	NLTOK amark;
   {
   nodeptr anode;
   bool    result;
   
   nodelock(node_to_augment);
   
   if (result=(*intokens==amark))
      {   
      popwordnode(anode,NLSFpool);

      set_node_token(anode,*intokens++);
      settail(node_to_augment,anode);
      }   
     
   nodeunlock(node_to_augment);
   
   return result;
   }
   /*end NLSF_PUNCT*/
   
bool NLSF_MUNCH(amark)

	NLTOK amark;
   {
   bool result;
   
   if (result=(*intokens==amark))
      intokens++;
      
   return result;
   }
   /*end NLSF_MUNCH*/
 
