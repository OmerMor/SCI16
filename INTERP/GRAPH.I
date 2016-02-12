;-------------------------------------------------------------------
;	graph.i
;	Data types and constants for SCI graphics routines
;-------------------------------------------------------------------
ifndef GRAPH_I
GRAPH_I equ	1

ifndef LIST_I
include list.i
endif

;; Structures ===================================================

;Elemental X/Y location
RPoint	struc		
	v				sword	?
	h				sword	?
RPoint	ends

;Rectangular enclosure of Points
RRect	struc
	top			sword	?
	left			sword	?
	bottom		sword	?
	right			sword	?
RRect	ends

RRectPtr	typedef ptr RRectPtr

;Font header structure
Font	struc
	lowChar		word	?
	highChar		word	?
	pointSize	word	?
	charRecs		word	?
Font	ends

;Standard view/loop/cel structures
; Revised for New (3/92) file formats

Cel	struc
	xDim			word	?
	yDim			word	?
	xOff			word	?
	yOff			word	?
	skipColor	byte	?
 	compressType	byte	?
	compRemapCount	word	?
	compressSize	dword	?
	controlSize	dword	?
	paletteOffset	dword	?
	dataOffset	dword	?
	colorOffset	dword	?
	compressRemapOffset dword	?
Cel	ends

Loop	struc
	altLoop		byte	?
	flags		byte	?
	numCels		byte	?
	dummy		byte	?
	startCel	byte	?
	endingCel	byte	?
	repeatCount	byte	?
	stepSize	byte	?
	paletteOffset	dword	?
	celOffset	dword	?
Loop	ends

View	struc
	viewHeaderSize	word	?
	numLoops		byte	?
	vFlags		byte	?
	hisplitFlag	byte	?
	dummy2		byte	?
	celCount	word	?
	paletteOffset	dword	?
	loopHeaderSize	byte	?
	celHeaderSize	byte	?
	animationOffset	dword	?
View	ends

Pic	struc
	picHeaderSize	dw	?
	nPriorities	db	?
	priLineCount	db	?
	celCount	db	?
	dummy		db	0		
	vanishX		dw	?
	vanishY		dw	?
	viewAngle	dw	?
	vectorSize	dd	?
	vectorOffset	dd	?
	priCelOffset	dd	?
	controlCelOffset dd	?
	paletteOffset	dd	?
	visualHeaderOffset	dd	?
	polygonOffset	dd	?
	priLine		dw	?
Pic	ends

;The graphport definition
RGrafPort	struc
	ptLink		Node		<>
	origin		RPoint	<>
	portRect		RRect		<>
	pnLoc			RPoint	<>
	fontSize		word 		8
	fontNum		word 		0
	txFace		word		?
	fgColor		word		?
	bkColor		word		?
	pnMode		word		?
RGrafPort	ends

GPPtr	typedef	ptr RGrafPort
RGrafPortPtr	typedef ptr RGrafPort


;; exports ======================================================

externdef	Display: 			dword
externdef	rThePort: 			ptr RGrafPort
externdef	theRect: 			RRect
externdef	wordBits: 			word
externdef	bounds:     		RRect
externdef	baseTable:  		word
externdef	vSeg:       		word
externdef	palVaryOn:        byte
externdef	palVaryPause:     byte
externdef	palettePercent:	word
externdef	paletteDir:			word
externdef	paletteTime:		word
externdef	paletteStop:		word
externdef	paletteRes:			word
externdef	startPalette:		word
externdef	targetPalette:		word
externdef	newPalette:			word
externdef	palServerSignal:	word
externdef  	NumberColors:     word
externdef	theFont:				memptr

; Buffers for dynamic scaling

externdef	vm_deltaxs:			word
externdef	vm_deltays:			word

; Global indicating that a resload should read
; data to the bottom of the buffer

externdef	lowData:				word

externdef	currentVideoMode:byte


SubmitPalette		proto	viewPtr:far ptr
PaletteUpdate		proto	flag:word, numberOfTimes:word
RSetFont			proto	font:word
ROpenPort		proto	gPort:GPPtr
CInitGraph		proto	driver:word
RGetPort			proto	pPtr:word
RSetPort			proto	pPtr:word
RSetOrigin		proto	x:word, y:word
RMoveTo			proto	x:word, y:word
RMove				proto	x:word, y:word
PenColor			proto	mode:word
RBackColor		proto	mode:word
RPenMode			proto	mode:word
RTextFace		proto	face:word
GetPointSize	proto
SetPointSize	proto	siz:word
GetFont			proto
CEndGraph		proto
RInvertRect		proto	r:ptr RRect
RPaintRect		proto	r:ptr RRect
REraseRect		proto	r:ptr RRect
RTextWidth		proto	s:ptr byte, first:word, cnt:word, defaultFont:word
RCharWidth		proto	cCode:word
CharHeight		proto	cCode:word
RDrawChar		proto	cCode:word
ShowChar			proto	cCode:word
JisChar			proto
JdrChar			proto	cCode:byte, kanji:byte
ROffsetRect		proto	r:ptr RRect, x:word, y:word
SOffsetRect		proto
RInsetRect		proto	r:ptr RRect, x:word, y:word
MoveRect			proto	r:ptr RRect, x:word, y:word
CSectRect		proto	r1:ptr RRect, r2:ptr RRect, r3:ptr RRect
RSectRect		proto
GetTheRect		proto
GetCursorData	proto	data:word
SetCursorData	proto	data:word


DrawPic			proto	hndl:word, clear:word, mirror:word
RFillRect		proto	rPtr:ptr RRect, lmapSet:word, color1:word, color2:word, color3:word 
SizeRect			proto	rPtr:ptr RRect
SaveBits			proto	rPtr:ptr RRect, lmapSet:word
RestoreBits		proto	hndl:word
UnloadBits		proto	hndl:word
SetCLUT			proto	palOff:word, palSeg:word, palClut:word
GetCLUT			proto	palOff:word, palSeg:word
ShowBits			proto	rPtr:ptr RRect, lmapSet:word
KGraph			proto	args:word
InitGraph		proto
EndGraph			proto
StdChar			proto
OnControl		proto	map:word, rPtr:ptr RRect
InitDisplay		proto
EndDisplay		proto	vMode:word
RShowCursor		proto
RHideCursor		proto
MoveCursor		proto	x:word, y:word
PointToCel		proto	view:word, loop:word, cel:word
PointToCel		proto	view:word, loop:word, cel:word
CopyCel			proto	skipCount:word, skipColors:word
RSetCursor		proto	view:word, loop:word, cel:word, hotX:word, hotY:word
ShakeScreen		proto	cnt:word, dir:word
ShiftScreen		proto	rtop:word, rleft:word, rbot:word, rright:word, dir:word
ColorFade		proto
PalDist			proto	red:word, green:word, blue:word
PalMatch			proto	red:word, green:word, blue:word

ResetPallette	proto near
ClearScreen		proto near


;; equates ======================================================

; pen transfer modes
SRCOR			equ	0
SRCCOPY		equ	1	; blanks entire character rectangle
srcInvert	equ	2
SRCAND		equ	3

;character styles
PLAIN			equ	0
DIMMED		equ	1
BOLDED		equ	2
INVERSE		equ	4

; basic pallete by numbers
vBLACK		equ	0
vWHITE		equ	0ffh	; high nibble is masked off on 16 color
vGRAY			equ	7

; pixel masks for virtual bitmaps
ODDON			equ	00fh
ODDOFF		equ	0f0h
EVENON		equ	ODDOFF
EVENOFF		equ	ODDON

; virtual BitMap selection equates
VMAP			equ	001h
PMAP			equ	002h
CMAP			equ	004h

; picdraw stuff
LOWCODE		equ	0f0h
ENDCODE		equ	0ffh
PENOFF		equ	0ffh

; airbrush bit in brushSize
AIRBRUSH		equ	020h

OFFSETLOOPTABLE	equ	8
OFFSETCELTABLE		equ	4
MAXCURSORBUF		equ	900


; KGraph function dispatch constants
GLoadBits		equ	1
GDetect			equ	2
GSetPalette		equ	3
GDrawLine		equ	4
GFillArea		equ	5   
GDrawBrush		equ	6
GSaveBits   	equ	7
GRestoreBits	equ	8
GEraseRect		equ	9
GPaintRect		equ	10
GFillRect		equ	11
GShowBits   	equ	12
GReAnimate		equ	13
GInitPri			equ	14

DOSETCLUT	equ	1
NOSETCLUT	equ	0

;;===============================================================

;; Macros =======================================================


SaveExtraBits	macro
	mov	bl, ah
	ror	bx, 1
	ror	bx, 1
	endm

GetExtraBits	macro
	xor	bl, bl
	rol	bx, 1
	rol	bx, 1
	mov	ah, bl
	endm

endif

