#ifndef QUERY_H

#define QUERY_H

/***************************************************************************

	#include HEADERS
	
	(to pick up external declarations for types,
	variables, functions, constants and macros)

***************************************************************************/

#ifndef	MEMORY_H
#include "memory.h"
#endif

#ifndef	SYMS_H
#include "syms.h"
#endif

/***************************************************************************

	external CONSTANT #DEFINITIONS

***************************************************************************/

#define NLQUERYSIZE 30
#define CLAUSESIZE 10


/***************************************************************************

	external MACRO #DEFINITIONS

***************************************************************************/


#define clause_head(c)   GetWord(*(c))
#define clause_tail(c)   ((c)+1)
#define empty_clause(c)  !*(c)

#define query_head(q)    GetWord(*(q))
#define query_tail(q)    ((q)+2)
#define empty_query(q)   !*(q)

#define clausedb_head(cdb) *(cdb)
#define clausedb_tail(cdb) ((cdb)+1)
#define EOGrammar(cdb)     empty_clause(clausedb_head(cdb))


#define querycpy(newq,oldq)      append_queries((memptr)NULL,oldq,newq)

#define query_head_slot(aquery)  GetWord((aquery)[0])
#define query_head_label(aquery) GetWord((aquery)[1])
		  
#define clause_expand(clause)    clause_tail(clause)
#define clause_query(clause)     clause_tail(clause)

#define stream_head(tokens)      *(tokens)
#define stream_tail(tokens)      ((tokens)+1)

#define no_more_tokens(intokens) !stream_head(intokens)
#define EOStream(intokens)       !stream_head(intokens)

/* To get SPEECHTAG out of:
   (label NLVOCAB SPEECHTAG)
   get the head of the tail of the tail
*/
#define clause_speech_tag(aclause) query_head_label(clause_query(aclause))



#define nlunify(spectags,realtags) (((spectags) & (realtags)) != 0x00)

#define nlunify_punct(specmark,realmark) \
        (((specmark)<<8==(realmark)) && ((specmark)<<8 & (realmark) & 0x8000))

#define punct_clause(c) (query_head_slot(clause_tail(c)) == NLPNCT)

#define word_query(q) (query_head_slot(q)==NLVOCAB)

#define word_clause(c)(word_query(clause_tail(c)))

#define word2_clause(c) (query_head_slot(clause_tail(c)) == NLPUT )

#define clause_match(c,q) (clause_head(c)==query_head_label(q))
/***************************************************************************

	external TYPEDEFS

***************************************************************************/

/*
   Definition of terms (by example):

      CLAUSE: VP,ROOT,VERB,DIROBJ,NP,NULL

		QUERY:  ROOT,VERB,DIROBJ,NP,NULL

		CLAUSEDB: an array of clauses, each one CLAUSESIZE tokens long

		STREAM: an array of wordnumbers
*/

typedef NLTOK far*        NLQUERY;
typedef NLTOK far         NLCLAUSE[CLAUSESIZE];
typedef NLCLAUSE far*     NLCLAUSEDB;

/***************************************************************************

	external VARIABLE DECLARATIONS

***************************************************************************/


/***************************************************************************

	external FUNCTION DECLARATIONS

***************************************************************************/


#ifdef LINT_ARGS

	extern bool append_queries(NLQUERY, NLQUERY, NLQUERY);  /* fill third query with

#else

	extern bool append_queries();  /* fill third query with info from first two */ 

#endif /* #ifdef-else LINT_ARGS */

/***************************************************************************

	END OF HEADER FILE

***************************************************************************/

#endif /* #ifndef _H */
