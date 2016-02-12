/* LIST
 * List manager.
 *
 * A list is a variable of type List:
 *
 *	List		theList;
 *
 * The elements of the list are structures whose first member is of
 * type Node:
 *
 *	struct	{
 *		Node	link;
 *		...
 *		}
 *
 * This structure is refered to in the following as an 'element' when not
 * in the list, and as a 'node' when linked into the list.
 *
 * The functions & macros are:
 *
 *	void	InitList(List *)
 *		Initialize the list as an empty list.  Note that calling this
 *		for a list which is not empty will render the elements
 *		inaccessible (at least through the list).
 *
 *	bool	EmptyList(List *)
 *		Returns TRUE if the list is empty, FALSE otherwise.
 *
 *	ObjID FirstNode(List *)
 *		Returns a pointer to the first node of the list.
 *
 *	ObjID LastNode(List *)
 *		Returns a pointer to the last node in a list.
 *
 *	ObjID NextNode(ObjID node)
 *		Returns a pointer to the next node in the list.
 *
 *	ObjID PrevNode(ObjID node)
 *		Returns a pointer to the previous node in the list.
 *
 *	bool	IsFirstNode(ObjID node)
 *		Returns TRUE if the node is the first node in the list.
 *
 *	bool	IsLastNode(ObjID node)
 *		Returns TRUE if the node is the last element in the list.
 *
 *	bool	DeleteNode(List *, ObjID node)
 *		Deletes the node from the list.  Returns (!EmptyList(list)).
 *
 *	ObjID AddAfter(List *, ObjID node, ObjID element, [key])
 *		Add the element to the list after the node.  Returns pointer to
 *		the inserted node.  If key is specified, sets the key.
 *
 *	ObjID AddBefore(List *, ObjID node, ObjID element, [key])
 *		Add the element to the list before the node.  Returns pointer to
 *		the inserted node.  If key is specified, sets the key.
 *
 *	ObjID AddToFront(List *, ObjID element, [key])
 *		Add the element to the beginning of the list.  Returns pointer to
 *		the inserted node.  If key is specified, sets the key.
 *
 *	ObjID MoveToFront(List *, ObjID node)
 *		Moves the node to the beginning of the list.  Returns pointer to
 *		the inserted node.
 *
 *	ObjID AddToEnd(List *, ObjID element, [key])
 *		Add the element to the end of the list.  Returns pointer to
 *		the inserted node.  If key is specified, sets the key.
 *
 *	ObjID MoveToEnd(List *, ObjID node)
 *		Moves the node to the end of the list.  Returns pointer to
 *		the inserted node.
 *
 *	void	SetKey(ObjID node, ObjID key)
 *		Sets the key for the node.
 *
 *	ObjID GetKey(ObjID node)
 *		Returns the key for the node.
 *
 *	ObjID FindKey(List *, ObjID key)
 *		Returns the first node in the list with the given key, or NULL
 *		if there is no node with that key.
 *
 *	bool	DeleteKey(List *, ObjID key)
 *		Delete the first node in the list with the given key.  Return
 *		TRUE if a node was found and deleted, FALSE otherwise.
 */


#include	"list.h"

#include	"stdarg.h"
#include	"memmgr.h"

global bool DeleteNode(list, node)
List		*list;
ObjID		node;
{
	if (IsFirstNode(node))
		FirstNode(list) = NextNode(node);
	else
		NextNode(PrevNode(node)) = NextNode(node);

	if (IsLastNode(node))
		LastNode(list) = PrevNode(node);
	else
		PrevNode(NextNode(node)) = PrevNode(node);

	return (!EmptyList(list));
}

ObjID
AddAfter(List* list, ObjID node, ObjID element, ...)
{
	va_list	key;

	va_start(key, element);

	if (IsLastNode(node)) {
		NextNode(element) = NULL;
		LastNode(list) = element;
		}
	else {
		NextNode(element) = NextNode(node);
		PrevNode(NextNode(node)) = element;
		}

	PrevNode(element) = node;
	NextNode(node) = element;

	((Node *)Native(element))->key = va_arg(key, uint);

	return (element);
}

ObjID
AddBefore(List* list, ObjID node, ObjID element, ...)
{
	va_list	key;

	va_start(key, element);

	if (IsFirstNode(node)) {
		PrevNode(element) = NULL;
		FirstNode(list) = element;
		}
	else {
		PrevNode(element) = PrevNode(node);
		NextNode(PrevNode(node)) = element;
		}

	NextNode(element) = node;
	PrevNode(node) = element;

	((Node *)Native(element))->key = va_arg(key, uint);

	return (element);
}

ObjID
AddToFront(List* list, ObjID element, ...)
{
	va_list	key;

	va_start(key, element);

	if (EmptyList(list)) {
		LastNode(list) = element;
		NextNode(element) = NULL;
		}
	else {
		PrevNode(FirstNode(list)) = element;
		NextNode(element) = FirstNode(list);
		}

	FirstNode(list) = element;
	PrevNode(element) = NULL;

	((Node *)Native(element))->key = va_arg(key, uint);

	return (element);
}



global ObjID MoveToFront(list, node)
List		*list;
ObjID		node;
{
	if (!IsFirstNode(node)) {
		/* Delete the node from its current position */
		NextNode(PrevNode(node)) = NextNode(node);
		if (IsLastNode(node))
			LastNode(list) = PrevNode(node);
		else
			PrevNode(NextNode(node)) = PrevNode(node);

		/* Add it to the start of the list. */
		PrevNode(FirstNode(list)) = node;
		NextNode(node) = FirstNode(list);
		FirstNode(list) = node;
		PrevNode(node) = NULL;
		}

	return (node);
}

ObjID
AddToEnd(List* list, ObjID element, ...)
{
	va_list	key;

	va_start(key, element);

	if (EmptyList(list)) {
		FirstNode(list) = element;
		PrevNode(element) = NULL;
		}
	else {
		NextNode(LastNode(list)) = element;
		PrevNode(element) = LastNode(list);
		}

	LastNode(list) = element;
	NextNode(element) = NULL;

	((Node *)Native(element))->key = va_arg(key, uint);

	return (element);
}



global ObjID MoveToEnd(list, node)
List		*list;
ObjID		node;
{
	if (!IsLastNode(node)) {
		/* Delete node from its current position.
		 */
		if (IsFirstNode(node))
			FirstNode(list) = NextNode(node);
		else
			NextNode(PrevNode(node)) = NextNode(node);
		PrevNode(NextNode(node)) = PrevNode(node);

		/* Add node to end of list.
		 */
		NextNode(LastNode(list)) = node;
		PrevNode(node) = LastNode(list);
		LastNode(list) = node;
		NextNode(node) = NULL;
		}

	return (node);
}



global ObjID FindKey(list, key)
List*	list;
ObjID		key;
{
	ObjID	node;

	if (list == NULL) return(NULL);
	for (node = FirstNode(list) ;
		node != NULL && GetKey(node) != key ;
		node = NextNode(node)
	    )
		    ;

	return (node);
}



global ObjID DeleteKey(list, key)
List		*list;
ObjID		key;
{
	ObjID	node;

	if ((node = FindKey(list, key)) != NULL)
		DeleteNode(list, node);

	return (node);
}

