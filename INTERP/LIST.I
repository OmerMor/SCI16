;-------------------------------------------------------------------
;	list.i
;-------------------------------------------------------------------

ifndef LIST_I
LIST_I	equ	1

NodePtr	typedef	ptr Node

List	struc
	lHead	NodePtr	?
	lTail	NodePtr	?
List	ends
	
Node	struc
	bak	NodePtr	?
	fwd	NodePtr	?
	key	sword		?
Node	ends

endif

