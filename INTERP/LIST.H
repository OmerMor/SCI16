// LIST.H

#if !defined(LIST_H)
#define LIST_H

#if !defined(TYPES_H)
#include "types.h"
#endif

typedef struct	{
	ObjID	next;				/* Pseudo-address of next node in list			*/
	ObjID	prev;				/* Pseudo-address of previous node in list	*/
	ObjID	key;
} Node;

typedef struct {
	ObjID	head;				/* Pseudo-address of head of list	*/
	ObjID	tail;				/* Pseudo-address of tail of list	*/
} List;

// function definition codes for kernel call ListOps 

#define	LEachElementDo	1	
#define	LFirstTrue		2
#define	LAllTrue			3

#define	FirstNode(list)		(((List *)list)->head)
#define	LastNode(list)			(((List *)list)->tail)
#define	InitList(list)		FirstNode((List *)list)=LastNode((List *)list)=NULL
#define	EmptyList(list)		(FirstNode((List *)list) == NULL)
#define	NextNode(node)			(((Node *)Native(node))->next)
#define	PrevNode(node)			(((Node *)Native(node))->prev)
#define	IsFirstNode(node)		(PrevNode(node) == NULL)
#define	IsLastNode(node)		(NextNode(node) == NULL)
#define	SetKey(node, aKey)	((Node *)Native(node))->key = aKey
#define	GetKey(node)			(((Node *)Native(node))->key)

bool		DeleteNode(List *, ObjID);
ObjID		AddAfter(List *, ObjID, ObjID, ...);
ObjID		AddBefore(List *, ObjID, ObjID, ...);
ObjID		AddToFront(List *, ObjID, ...);
ObjID		MoveToFront(List *, ObjID);
ObjID		AddToEnd(List *, ObjID, ...);
ObjID		MoveToEnd(List *, ObjID);
ObjID		FindKey(List *, ObjID);
ObjID		DeleteKey(List *, ObjID);

#endif
