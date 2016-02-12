;-------------------------------------------------------------------
;	mouse.i
;-------------------------------------------------------------------

ifndef MOUSE_I

MOUSE_I	equ	1

ifndef GRAPH_I
include graph.i
endif

externdef mouseX:word
externdef mouseY:word
externdef haveMouse:word
externdef mouseIsDebug:word
externdef restrictRecPtr:RRectPtr
externdef restrict:RRect
externdef buttonState:word

CurMouse			proto	mpoint:ptr Point
DisposeMouse	proto
InstallMouse	proto
RGetMouse		proto	mpoint:ptr Point
SetMouse			proto	mpoint:ptr Point

endif

