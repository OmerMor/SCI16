;-------------------------------------------------------------------
;	cels.i
;-------------------------------------------------------------------

ifndef	CELS_I

CELS_I	equ	1

ifndef	GRAPH_I
include	graph.i
endif

DrawCel			proto	view:word, loop:word, cel:word, pt:ptr RPoint, pri:word
DrawCelZ			proto
DrawPicCel		proto
ScaleDrawCel	proto	view:word, lop:word , cel:word, pt:ptr RPoint, pri:word,
					realXDim:word
GetCelWide		proto	view:word, loop:word, cel:word
GetCelHigh		proto	view:word, loop:word, cel:word
GetCelXOff		proto	view:word, loop:word, cel:word
GetCelYOff		proto	view:word, loop:word, cel:word
GetCelRect		proto	view:word, loop:word, cel:word, x:word, y:word, z:word, rPtr:ptr RRect
GetNumLoops		proto	view:word
GetNumCels		proto	view:word, loop:word
RIsItSkip		proto	view:word, loop:word, cel:word, vOffset:word, hOffset:word
SetNewDim		proto view:word, lop:word,  cel:word, newxdim:word, newydim:word, newxoff:word, newyoff:word
DrawMagCursor	proto	magData:ptr MagCursorStruct,pCursor:ptr pseudoCursor, xpos:word, ypos:word
AssertViewPalette	proto	view:word, flag:word
SetupVMap 		proto sourcePage:word

REMAPCOLORSTART equ	0fdh
REMAPCOLOREND  equ	0feh

endif

