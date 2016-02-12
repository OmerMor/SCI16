;-------------------------------------------------------------------
;animate.i
;-------------------------------------------------------------------
ifndef ANIMATE_I
ANIMATE_I EQU 1

ifndef GRAPH_I
include graph.i
endif

ifndef LIST_I
include list.i
endif

externdef 	lastCast:word

DisposeLastCast	proto
ReAnimate			proto		r:ptr Rect
Animate				proto		l:ptr List, doit:word
AddToPic				proto		l:ptr List

endif
