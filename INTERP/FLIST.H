//	flist.h

#if !defined(FLIST_H)
#define FLIST_H

#if !defined(MEMMGR_H)
#include	"memmgr.h"
#endif

typedef struct	{
	word	head;
	word	tail;
} FList;

typedef struct {
	word	prev;
	word	next;
	int	key;
} FNode;

typedef	FNode _far*	FNodePtr;
typedef	FNodePtr*	FNodeHandle;

#define	W2H(w)	((FNodeHandle) Native(w))	// word to handle
#define	H2W(n)	((word) Pseudo(n))			// handle to word

bool	FIsFirstNode(word node);
word	FFirstNode(FList* list);
word	FLastNode(FList* list);
word	FNextNode(word node);
word	FPrevNode(word node);
word	FAddToFront(FList* list, word node);
bool	FDeleteNode(FList* list, word node);
word	FMoveToFront(FList* list, word node);

#endif
