#ifndef LISTS_H

#define LISTS_H



#ifdef notdef

#include "gc.h" /* garbage-collection for lists */

#endif



#ifndef GC_H /* use rest of this file if no GC */

/***************************************************************************

	#include HEADERS
	
	(to pick up external declarations for types,
	variables, functions, constants and macros)

***************************************************************************/

#ifndef	TYPES_H
#include "types.h"
#endif

#ifndef	SYMS_H
#include	"syms.h"
#endif


/***************************************************************************

	external CONSTANT #DEFINITIONS

***************************************************************************/

#define virgintype  (nodetype)1
#define emptytype   (nodetype)2
#define tagtype     (nodetype)3
#define wordtype    (nodetype)4
#define tokentype   (nodetype)5
#define listtype    (nodetype)6
#define endofpool   (nodetype)7


/***************************************************************************

	external MACRO #DEFINITIONS

***************************************************************************/




/**************************************
      Generic NLNODE macros
**************************************/

#define nodeassign(var,np)      var=np                            
#define nodelock(np)            /* no-op if no GC used */
#define nodeunlock(np)          /* no-op if no GC used */
#define noderelease(np)         /* no-op if no GC used */
#define mark_and_sweep(a_uint) /* no-op if no GC used */

#define head(np)               (np)->data.list.head
#define tail(np)               (np)->data.list.tail
#define sethead(np,np2)        (np)->data.list.head=np2
#define settail(np,np2)        (np)->data.list.tail=np2

#define set_node_tags(np,tags) (np)->data.tags  = tags
#define get_node_tags(np)      (np)->data.tags

#define set_node_token(np,tok) (np)->data.token = tok
#define get_node_token(np)     (np)->data.token

#define getnodetype(nptr)       ((nodetype)((nptr) ? (nptr)->type : NULL))
#define setnodetype(nptr,ntype) (nptr)->type=ntype

#define nullp(np)  (!np || getnodetype(np)==emptytype )
#define consp(np)  (getnodetype(np)==listtype)
#define tokenp(np) (getnodetype(np)==tokentype)
#define wordp(np)  (getnodetype(np)==wordtype)
#define listp(np)  (nullp(np) || consp(np))
#define virginp(np)(getnodetype(np)==virgintype)

/**************************************
      LISP-ish aliases
**************************************/

#define rplaca(np1,np2) sethead(np1,np2)
#define rplacd(np1,np2) settail(np1,np2)

#define car(nodeptr) head(nodeptr)
#define cdr(nodeptr) tail(nodeptr)

#define caar(np) car(car(np))
#define cadr(np) car(cdr(np))
#define cdar(np) cdr(car(np))
#define cddr(np) cdr(cdr(np))

/**************************************
      NLP tree-access macros
**************************************/

#define treelistp(np)      (consp(np) && consp(head(np)))
#define treetail(np)       (tail(tail(np)))

#define terminal_nodep(np) !listp(tail(tail(np)))
#define nodeslot(np)       get_node_token(head(np))
#define nodelabel(np)      get_node_token(head(tail(np)))
#define termslot(np)       nodeslot(np)
#define termlabel(np)      nodelabel(np)
#define termdatum(np)      get_node_token(tail(tail(np)))

/**************************************
      Pool-based NLNODE macros
**************************************/
#define EOPool(nodeptr) getnodetype(nodeptr)==endofpool

#define popnode(newnp,npool) \
        {newnp=npool++;      \
        if (EOPool(newnp)) return FALSE;}
                
#define poplistnode(newnodeptr,nodepool)                 \
        {popnode(newnodeptr,nodepool);                   \
         setnodetype(newnodeptr,listtype);}

#define poptagnode(newnodeptr,nodepool)                  \
        {popnode(newnodeptr,nodepool);                   \
         setnodetype(newnodeptr,tagtype);}

#define poptokennode(newnodeptr,nodepool)                \
        {popnode(newnodeptr,nodepool);                   \
         setnodetype(newnodeptr,tokentype);}

#define popwordnode(newnodeptr,nodepool)                 \
		  {popnode(newnodeptr,nodepool);                   \
			setnodetype(newnodeptr,wordtype);}


#define nlcons(newnp,np1,np2,nodepool)                   \
        {poplistnode(newnp,nodepool);                    \
         sethead(newnp,np1);                             \
         settail(newnp,np2);}


/***************************************************************************

	external TYPEDEFS

***************************************************************************/

typedef ubyte nodetype;

/*
	node: this is the central general-purpose data structure
	      in particular, listnodes can have their head and tail point to
	      anything at all. For standard lists only the head of a listnode
	      should point at non-listnodes
*/

struct node {
   nodetype                          type;
   union nodedata {
   	NLTAGS                  tags;
   	NLTOK                   token;
   	struct listnode {
   		struct node far *head;
   		struct node far *tail;
         }                    list;
      }                              data;
   };
   
typedef struct node far      node;
typedef node*                nodeptr;

/***************************************************************************

	external VARIABLE DECLARATIONS

***************************************************************************/

extern int LISTDEBUG;
extern int NLDEBUG;

/***************************************************************************

	external FUNCTION DECLARATIONS

***************************************************************************/


#ifdef LINT_ARGS

	extern void    init_nodepool(nodeptr,uint);
	extern nodeptr assoc(nodeptr, NLTOK);

#else

	extern void    init_nodepool();
	extern nodeptr assoc();

#endif /* #ifdef-else LINT_ARGS */

/***************************************************************************

	END OF HEADER FILE

***************************************************************************/

#endif /* #ifndef GC_H */

#endif /* #ifndef NLLISTS_H */
