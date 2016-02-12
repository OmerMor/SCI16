;-------------------------------------------------------------------
;	dialog.i
;-------------------------------------------------------------------

ifndef	DIALOG_I

DIALOG_I		equ	1

ifndef	OBJECT_I
include	object.i
endif

ifndef	GRAPH_I
include	graph.i
endif

ifndef	EVENT_I
include	event.i
endif


RHiliteControl	proto		s:ptr Obj
DrawControl		proto		s:ptr Obj
EditControl		proto		s1:ptr Obj, s2:ptr Obj
GetLine			proto		s1:ptr byte, s2:ptr byte, i:word
RGlobalToLocal	proto		p:ptr Point
RLocalToGlobal	proto		p:ptr Point
EditText			proto		r:ptr Rect, s:ptr byte, i1:word, i2:word, e:ptr Event
DrawCursor		proto		r:ptr Rect, s:ptr byte, i:word
EraseCursor		proto


endif

