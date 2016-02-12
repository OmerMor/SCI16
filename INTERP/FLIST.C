/*
 *	flist.c
 *
 * List manager for far lists.
 */


#include	"flist.h"
#include	"memmgr.h"
#include	"intrpt.h"



bool
FIsFirstNode(word node)
// Return true if this node is first in list (no previous node).
{
	word	prev;

	_cli();
	prev = (*W2H(node))->prev;
	_sti();

	return  prev == NULL;
}



word
FFirstNode(FList *list)
// Returns first node in list (value in list.lHead will be zero for empty list).
{
	return list->head;
}



word
FLastNode(FList *list)
// Returns last node in list (value in list.lTail will be zero for empty list).
{
	return list->tail;
}



word
FNextNode(word node)
// Return handle to next node in list.
{
	word	next;

	_cli();
	next = (*W2H(node))->next;
	_sti();

	return next;
}



word
FPrevNode(word node)
// Return handle to previous node in list.
{
	word	prev;

	_cli();
	prev = (*W2H(node))->prev;
	_sti();

	return prev;
}



word
FAddToFront(FList *list, word node)
// Add this element to front of list and return the node added.
{
	FNodePtr	np;
	
	_cli();

	np = *W2H(node);
	np->next = list->head;
	np->prev = NULL;

	if (list->tail == NULL)
		// An empty list.  The node is the new tail.
		list->tail = node;

	if (list->head != NULL)
		(*W2H(list->head))->prev = node;

	list->head = node;

	_sti();

	return node;
}



bool
FDeleteNode(FList *list, word node)
// Delete 'node' from the list.  Return TRUE if the list still has elements.
{
	FNodePtr	np;
	
	_cli();

	np = *W2H(node);

	if (node == list->head)
		list->head = np->next;
	if (node == list->tail)
		list->tail = np->prev;

	if (np->next)
		(*W2H(np->next))->prev = np->prev;
	if (np->prev)
		(*W2H(np->prev))->next = np->next;

	_sti();

	return list->head != NULL;
}



word
FMoveToFront(FList *list, word node)
// Move this node to the front of the list.
{
	if (list->head != node) {
		FDeleteNode(list, node);
		FAddToFront(list, node);
		}

	return node;
}

