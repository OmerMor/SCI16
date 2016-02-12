;-------------------------------------------------------------------
; graph256.s
; Core graphics (320 X 200) module in support of interpreter
;-------------------------------------------------------------------

include macro.i
include graph256.i

include graph.i
include cels.i
include video.i
include driver.i
include resource.i
include restypes.i
include memmgr.i
include picture.i
include palette.i
include pmachine.i
include animate.i
include start.i
include stdlib.i
include intrpt.i
include errmsg.i

; for air brushing 
RANDMASK		equ	0b8h


.DATA

bounds		RRect	<0,0,200,320>
wholeScreen	RRect	<0,0,200,320>
baseTable	word	200 dup (?)

NumberColors     dw 0

;dimMask		byte	0aah,055h,0aah,055h,0aah,055h,0aah,055h
;penByte		word	?
showRect 	RRect	<>

; segments of the virtual bitmaps
vSeg			word	?
pcSeg			word	?	; this one is artificial

; handles to virtual bitmaps (save to dispose of at endgraph)
vHndl			word	?
pcHndl		word	?

;oldCode		word	0	; 

hRun			word	?
vRun			word	?

; the following table remaps logical vColor to physical color
DK			equ	2
cLIGHT	equ	3

; 256 color base constants
DKGRAY	equ	3
LTGRAY	equ	5
RED		equ	(1*8)+DK
ORANGE	equ	(2*8)+DK
YELLOW	equ	(3*8)+DK
GREEN		equ	(4*8)+DK
CYAN		equ	(5*8)+DK
BLUE		equ	(6*8)+DK
VIOLET	equ	(7*8)+DK

; Defines for ModeXH Horizontal Screen Locations
PAGE0		equ	0
PAGE1		equ	80
PAGE2		equ	160
PAGE3		equ	240

; this table remaps priority/control (replicates low nibble)

;nibbles	label	byte
;	byte	000h, 011h, 022h, 033h, 044h, 055h, 066h, 077h
;	byte	088h, 099h, 0aah, 0bbh, 0cch, 0ddh, 0eeh, 0ffh



; variables for picdraw

picPtr		dword	?	; far picture data pointer
palPtr		dword	?	; far palette data pointer
lineStartY	sword	?
lineStartX	sword	?
;lineEndY		sword	?
;lineEndX		sword	?

theFont		memptr	?

funcTable	label	word
	dword	SetColor			;f0
	dword ClearColor		;f1
	dword SetPriority		;f2
	dword ClearPriority	;f3
	dword DoShortBrush	;f4
	dword DoMedLine		;f5
	dword DoAbsLine		;f6
	dword DoShortLine		;f7
	dword DoAbsFill		;f8
	dword SetBrSize		;f9	used to be brush
	dword DoAbsBrush		;fa
	dword SetControl		;fb
	dword ClearControl	;fc
	dword DoMedBrush		;fd
	dword DoSpecial		;fe

specTable	label	word
	dword	OldSetPalette	; 00
	dword	DoBitMap			; 01
	dword	SetCLUTPalette	; 02
	dword	SetPriTable		; 03	Set up pri bands based on top bottom value
	dword	SetPriBands		; 04	Each pri band in table is specified in data

; working variables for Line
penY			sword		?
penX			sword		?
endY			sword		?
endX			sword		?
;restartX		word		?
;theMask		word		?
; working variables for FillArea
;backSeg		word		0				; guide segment for fill
mapSet		word		0				; active vMaps
;brushBit		word		0


FILL_STACK_SIZE	equ	4000		; Space for Fill pseudo-stack
fillStackHandle	word	0
fillStackSeg		word	0
fillStackBase		word	0

rightSide		sword		?
leftSide			sword		?
oldRightSide	sword		?
oldLeftSide 	sword		?
xScan				sword		?
yScan				sword		?
;theCaller		word		?


; plotting masks
;onMask		      byte		0
;offMask		      byte		0
;dots			      byte		0f0h, 00fh
;backColor	      byte		0
;fillColor	      byte		0	; guide segment fill color


;background colors for each bitmap
;vBack			   	byte		0ffh
;pBack			   	byte		0
;cBack			   	byte		0

vColor		      byte		?
pColor		      byte		?
cColor		      byte		?
;vNibbles		   	byte		?
;pNibbles		   	byte		?
;cNibbles		   	byte		?
;colorNibs	      byte		?

;mirroring flags
mirrorX	         word	0
;mirrorY	         word	0

;badCodeMess byte	"Bad picture code", 0

picOverlayMask		word	?



.CODE

DoCel	proto	theHandle:word

RFillRect proc	uses si di bp,	rPtr:ptr RRect, lmapSet:word, color1:word,\
									color2:word, color3:word
LOCAL trMode:word
; Draw the rectangle on screen in the requested bitMaps in requested colors
; Colors passed are expect in v/p/c order!
; Dummies must be passed for any skipped map

	push	ds
	pop	es

	; copy input rectangle to local storage
	mov	si, rPtr
	lea	di, theRect
	mov	cx, 4
	rep	movsw

	; set local transfer mode
	mov	di, rThePort
assume	di:ptr RGrafPort
	mov	ax, [di].pnMode
	mov	trMode, ax

	; clip to current portRect
	lea	di, [di].portRect
	lea	si, theRect
	mov	bx, si
	call	RSectRect
	jl		@F
	jmp	FRExit
@@:

	; make it global
	mov	di, rThePort
	call	SOffsetRect								; theRect still at SI
assume	di:nothing

	; set up base address of first line and vertical runcount
	mov	dx, theRect.bottom
	mov	di, theRect.top
	sub	dx, di
	mov	vRun, dx
	Base	di, baseTable

	; determine horizontal run and remainder of rowBytes
	mov	cx, theRect.right
	mov	bx, theRect.left
	sub	cx, bx
	mov	hRun, cx
	add	di, bx

	; produces the remainder of byte to advance at end of run
	mov	bx, VROWBYTES
	sub	bx, cx

	; now determine bitMap type and proceed accordingly
	.if	lmapSet & VMAP
		; do all lines
		push	di
		mov	es, vSeg
		mov	dx, vRun
		mov	al, byte ptr color1
		mov	ah, byte ptr color2
		.repeat
			mov	cx, hRun							; reload runcount
			.if	trMode == srcInvert
				.repeat
					.if	es:[di] == al
						mov	es:[di], ah
					.elseif es:[di] == ah
						mov	es:[di], al
					.endif
					inc	di
				.untilcxz
			.else
				rep stosb
			.endif
			add	di, bx							; advance DI to next line
			dec	dx									; decrement vertical run count
		.until zero?
		pop	di										; restore DI for next rectangle
	.endif

	; some slight optimization for pri/control maps
	mov	es, pcSeg
	mov	dx, vRun
	and	word ptr lmapSet, CMAP or PMAP
	.if	word ptr lmapSet == (CMAP or PMAP)
		; do two maps at once
		; merge priority and control into AL
		mov	al, byte ptr color2
		shl	al, 1
		shl	al, 1
		shl	al, 1
		shl	al, 1
		or		ax, color3
		.repeat
			mov	cx, hRun							; reload runcount
			.if	trMode == srcInvert
				.repeat
					xor	es:[di], al
					inc	di
				.untilcxz
			.else
				rep stosb
			.endif
			add	di, bx							; advance DI to next line
			dec	dx									; decrement vertical run count
		.until zero?
	.else
		; are we doing either?
		.if	byte ptr lmapSet & (PMAP or CMAP)
			; set up color and mask in proper nibbles
			.if	byte ptr lmapSet & PMAP
				mov	al, byte ptr color2
				shl	al, 1
				shl	al, 1
				shl	al, 1
				shl	al, 1
				mov	ah, EVENOFF
			.else
				mov	ax, color3
				mov	ah, ODDOFF
			.endif

			.repeat
				mov	cx, hRun						; reload runcount
				.if	trMode == srcInvert
					.repeat
						xor	es:[di], al
						inc	di
					.untilcxz
				.else
					.repeat
						and	es:[di], ah
						or		es:[di], al
						inc	di
					.untilcxz
				.endif
				add	di, bx						; advance DI to next line
				dec	dx								; decrement vertical run count
			.until zero?
		.endif
	.endif

FRExit:
	ret

RFillRect endp



ClearScreen	proc	near
;Erase all VMAPS to background color
	
	mov	bx, rThePort
assume	bx:ptr RGrafPort
	invoke	RFillRect, addr [bx].portRect, VMAP+PMAP+CMAP, vWHITE, 0, 0
assume	bx:nothing
	ret

ClearScreen endp



DrawPic proc uses es si di, hndl:word, clear:word, mirror:word
;Draw the picture described by data at hndl, clear is boolean

	push	ds
	pop	es

	;set picNotValid to initiate a show
	mov	picNotValid, 1
	
	; if FALSE we are doing an overlay
	.if	clear & TRUE
		mov	picOverlayMask, 0	; force 0 priority
		call	ClearScreen
	.else
		mov	picOverlayMask, 0fh	; allow any VALID priority
	.endif

	;clear mirroring flags
	mov	ax, mirror
	mov	mirrorX,0
;	mov	mirrorY,0

	;are we mirroring this picture?
	.if	ax & HMIRROR
		mov	mirrorX,TRUE
	.endif

; if we need vertical mirroring
;	.if	ax & VMIRROR
;		mov	mirrorY,TRUE
;	.endif

	invoke	ResLock, RES_PIC, hndl, TRUE	; Lock picture in memory
	; invoke LockHandle, hndl

	mov	si, hndl
	mov	ax, [si+_seg]
	mov	es, ax
	mov	si, [si+_off]

	push	es
	push	si

	call	SetPriBands			; requires es::si set to picture

	pop	si
	pop	es

	invoke	DoCel, hndl				; Draw the picture's bitmap

	; Allocate a MEMORY resource for Fill pseudo-stack
	; We will use es:bp to access the frame (in the Fill routines)
	invoke	ResLoad, RES_MEM, FILL_STACK_SIZE
	mov		fillStackHandle,ax
	.if		!ax
		invoke Panic, E_DRAWPIC
	.endif

	; set default color/priority/control
	mov	vColor, -1
	mov	pColor, -1
	mov	cColor, -1
;	mov	oldCode, 0								; default to newCodes

	; dispatch to first code
	
	mov	si, hndl
	mov	ax, [si+_seg]
	mov	word ptr picPtr+_seg, ax
	mov	es, ax
	mov	ax, [si+_off]
	mov	si, ax	 	
assume si:ptr Pic
	add	ax, word ptr es:[si].vectorOffset
	mov	word ptr picPtr+_off, ax

; set picPtr to point to the vector table
assume si:nothing	

	call	DispatchGrf									; returns only when complete

	; Release the memory used for pseudo-stack
	mov		ax, fillStackHandle
	invoke	ResUnLoad, RES_MEM, ax
	invoke	ResLock, RES_PIC, hndl, FALSE	; Unlock the picture

	;clear mirroring flags
	mov	mirrorX,0
;	mov	mirrorY,0

	ret

DrawPic	endp


DispatchX:
; some routines jump here to inc picPtr
	inc	word ptr picPtr
DispatchGrf:
; get picture codes and jump to proper handler
	les	si, picPtr
	inc	word ptr picPtr	; bump pointer word
	mov	bl, es:[si]
DispatchZ:
; some routines jump here with code in BL and pre-inced picPtr
	.if	bl == ENDCODE
		return
	.endif
	and	bl, 0fh
; use BL to index jump table
	xor	bh, bh
	shl	bx, 1
	shl	bx, 1
	jmp	dword ptr funcTable[bx]
; all handlers gather their own parms and return via jmp



DoSpecial:
; this is an escape function. The following byte determines
; the actual function invoked.
	les	si, picPtr
	inc	word ptr picPtr
	mov	bl, es:[si]
	xor	bh, bh
	shl	bx, 1
	shl	bx, 1

	; all functions will return via normal convention
	jmp	dword ptr specTable[bx]



SetPriTable	proc	near
;Get new values to build priority tables
SetPriTable	endp



SetPriBands	proc	near uses si
;Each priority band to be set in the table is specified in data
	
	; es:si now pointing to picture header

	add		si, Pic.priLine
	invoke	PriBands, es::si
	ret

SetPriBands	endp



SetCLUTPalette	proc	near
;Data at picPtr looks just like a RPalette
;Pass it in AX:SI to RSetPalette

;	mov	oldCode, 0
	les	si, picPtr								; advance picPtr past embedded palette data
	mov	word ptr palPtr+_off, si			; save this palette pointer
	mov	word ptr palPtr+_seg, es
	add	word ptr picPtr, PAL_FILE_SIZE
	invoke	RSetPalette, es::si, PAL_REPLACE
	jmp	DispatchGrf

SetCLUTPalette	endp


DoBitMap proc near
	jmp	DispatchGrf
DoBitMap endp


DoCel	proc uses si, theHandle
local PalPtr:Dword

	; es:si point to the picture header

assume si:ptr Pic

	.if !(es:[si].celCount)
		ret
	.endif

	xor	bh, bh
	mov	bl, es:[si].priLineCount
	shl	bx, 1								; normaly 16 * 2
	mov	dx, word ptr es:[si][bx].priLine 		; get the cel's priority
	push	dx

	invoke	RHideCursor

	mov	si, theHandle
	les	si, dword ptr [si]
	push	si						; need picture offset for DrawPicCel

	mov	di, si				; for use by RSetDanPalette
	add	di, word ptr es:[si].paletteOffset
	mov   word ptr PalPtr, di
	mov   ax, es
	mov   word ptr PalPtr+2, ax
	lea   si, PalPtr


; the following call is safe only as long as RSetDanPalette
; does not allocate memory or load resources! (which it currently doesn't)
	invoke	RSetDanPalette, si

	mov	si, theHandle
	les	si, dword ptr [si]
	add	si, word ptr es:[si].visualHeaderOffset

assume si:ptr Cel

	mov	ax, es:[si].xOff
	mov	bx, es:[si].yOff
	mov	di, mirrorX
	pop	cx					; the picture's offset
	pop	dx					; the cel's priority value
	and	dx, picOverlayMask  	; will mask out priority for non-overlay pics

	; Setup for DrawPicCel now done with:
	; ax = xOffset
	; bx = yOffset
	; cx = picture offset
	; dx = pic's priority
	; si = picture's cel offset
	; di = mirror flag
	; es = picture's segment
	
	call	DrawPicCel
	invoke	RShowCursor

	ret
assume si:nothing

DoCel	endp



SetColor	proc	near

	call	GetByte
	mov	vColor, bl
	jmp	DispatchGrf

SetColor	endp



SetPriority	proc	near

	call	GetByte
	shl	bl, 1
	shl	bl, 1
	shl	bl, 1
	shl	bl, 1
	mov	pColor, bl
	jmp	DispatchGrf

SetPriority	endp



SetControl	proc	near

	call	GetByte
	mov	cColor, bl
	jmp	DispatchGrf

SetControl	endp



ClearColor	proc	near

	mov	vColor, PENOFF
	jmp	DispatchGrf

ClearColor	endp



ClearPriority	proc	near

	mov	pColor, PENOFF
	jmp	DispatchGrf

ClearPriority	endp



ClearControl	proc	near

	mov	cColor, PENOFF
	jmp	DispatchGrf

ClearControl	endp



DoShortLine	proc	near
;Get absolute (3 byte) start and subsequent packed bytes 

	call	GetAbsXY
	mov	lineStartX, ax
	mov	lineStartY, bx

	.while TRUE
		call	GetShortXY

		; get endpoints into proper registers
		add	ax, lineStartX
		mov	cx, ax
		add	bx, lineStartY
		mov	dx, bx

		; update next lineStart and set lineStart to lineEnd
		mov	ax, lineStartX
		mov	bx, lineStartY
		mov	lineStartX, cx
		mov	lineStartY, dx

		; draw the line
		call	Line
	.endw

DoShortLine	endp



DoMedLine	proc	near
;Get first coords and loop getting more until function code encountered

	call	GetAbsXY
	mov	lineStartX, ax
	mov	lineStartY, bx
	.while TRUE
		call	GetMedXY

		; get endpoints into proper registers
		add	ax, lineStartX
		mov	cx, ax
		add	bx, lineStartY
		mov	dx, bx

		; update next lineStart and set lineStart to lineEnd
		mov	ax, lineStartX
		mov	bx, lineStartY
		mov	lineStartX, cx
		mov	lineStartY, dx

		; draw the line
		call	Line
	.endw

DoMedLine	endp



DoAbsLine	proc	near
;Get first coords and loop getting more until function code encountered

	call	GetAbsXY
	mov	lineStartX, ax
	mov	lineStartY, bx
	.while TRUE
		call	GetAbsXY
		; get endpoints into proper registers
		xchg	ax, cx
		xchg	bx, dx
		mov	ax, lineStartX
		mov	bx, lineStartY
		mov	lineStartX, cx
		mov	lineStartY, dx
		call	Line
	.endw

DoAbsLine	endp



DoAbsFill	proc	near
;Get coords until function code encountered

	.while TRUE
		call	GetAbsXY
		call	Fill
	.endw

DoAbsFill	endp



NoFunc:
; Bad code encountered
	invoke	exit, 1



GetByte	proc	near
;Return next byte of picture data in BL
;Jump to DispatchZ if a function code is found

	les	si, picPtr
	inc	word ptr picPtr
	mov	bl, es:[si]
	.if	bl >= LOWCODE
		add	sp, 2									; discard return address
		jmp	DispatchZ
	.endif
	ret

GetByte	endp



GetShortXY	proc	near
;Get next byte and unpack it into signed offset in AX/BX

	les	si, picPtr
	mov	bl, es:[si]
	inc	word ptr picPtr
	.if	bl >= LOWCODE
		add	sp, 2									; discard return address
		jmp	DispatchZ							; code in BL
	.endif

	push	cx
	mov	cl, bl
	mov	al, cl

	; do Y (low nibble) first
	and	al, 0fh
	.if	al & 08h
		and	al, 07h
		neg	al
	.endif
	cbw
	mov	bx, ax

	; now do X (hi nibble)
	mov	al, cl									; original byte
	shr	al, 1
	shr	al, 1
	shr	al, 1
	shr	al, 1
	.if	al & 08h
		and	al, 07h
		neg	al
	.endif
	cbw
	pop	cx
	ret

GetShortXY	endp



GetMedXY		proc	near
;Next two bytes are signed offsets from current (Y is first byte)
;Only first byte may be the function code
;Return X in AX, Y in BX
;The byte containing Y must be twiddled into twos complement

	les	si, picPtr
	mov	bl, es:[si]
	inc	word ptr picPtr
	.if	bl >= LOWCODE
		add	sp, 2									; discard return address
		jmp	DispatchZ							; code in BL
	.endif

	; make the Y
	mov	al, bl
	.if	al & 80h
		and	al, 07fh
		neg	al
	.endif
	cbw
	mov	bx, ax									; return Y in this register

	; get the X
	les	si, picPtr
	mov	al, es:[si]
	inc	word ptr picPtr
	cbw
	
	; X in AX, Y in BX
	ret

GetMedXY	endp



GetAbsXY	proc	near
;Unpack next 3 bytes of picture code into a X/Y pair
;Only first byte may be function code

	xor	ax, ax
	xor	dx, dx									; build Y here
	les	si, picPtr
	mov	bl, es:[si]
	inc	word ptr picPtr
	.if	bl >= LOWCODE
		add	sp, 2									; discard return address
		jmp	DispatchZ
	.endif

	; this byte has MSBits of X in upper nibble and MSBits of Y in low nibble
	mov	dh, bl
	and	dh, 0fh
	shr	bl, 1
	shr	bl, 1
	shr	bl, 1
	shr	bl, 1
	mov	ah, bl

	; get LSBits of X
	les	si, picPtr
	inc	word ptr picPtr
	mov	bl, es:[si]
	mov	al, bl
	
	; get LSBits of Y and put in BX
	les	si, picPtr
	inc	word ptr picPtr
	mov	bl, es:[si]
	mov	dl, bl
	mov	bx, dx
	ret

GetAbsXY	endp



SetFastDot	proc	near
;Setup for FastDot, which is for constant color dots at the same y coordinate.

	mov	si, penY
	Base	si, baseTable
	return

SetFastDot	endp


DoDot	proc	near

	FastDot
	return

DoDot	endp




assume	di:ptr byte


HLine	proc	near
;Draw a strictly horizontal line

assume	bx:sword
assume	cx:sword

	call	SetFastDot
	mov	bx, penX
	mov	cx, endX

	;Make sure that we're drawing the line left to right.
	.if	bx > cx
		xchg	bx, cx
	.endif

	; needed to complete the line
	inc	cx
	call	FillLines
		
assume	bx:nothing
assume	cx:nothing

	return

HLine	endp



VLine	proc	near
;Draw a strictly vertical line from lesser to greater

assume	ax:sword
assume	cx:sword

	mov	ax, penY
	mov	cx, endY

	;Make sure that we're drawing the line top to bottom
	.if	ax > cx
		xchg	ax, cx
		mov	penY, ax
		mov	endY, cx
	.endif
	sub	cx, ax
	inc	cx											;cx = # of points to plot

	;Get offset of top point.
	call	SetFastDot
	mov	bx, penX
	lea	di, [bx][si]			;points at byte containing point

	;Get the mapset in bx
	mov	bx, mapSet

	; pcSeg is default ES to minimize reloading
	mov	es, pcSeg
	.repeat
		.if	bx & VMAP
			mov	es, vSeg
			mov	al, vColor
			mov	es:[di], al						; get current data
			mov	es, pcSeg
		.endif

		.if	bx & PMAP
			mov	al, pColor
			and	es:[di], EVENOFF
			or		es:[di], al
		.endif

		.if	bx & CMAP
			mov	al, cColor
			and	es:[di], ODDOFF
			or		es:[di], al
		.endif

		;move to next line down
		add	di, VROWBYTES
	.untilcxz

	return

assume	ax:nothing
assume	cx:nothing

VLine	endp



DLine	proc	near
; determine X/Y vector (direction of change) and pseudo fractional delta
LOCAL xVector:sword, yVector:sword, xDelta:sword, yDelta:sword

assume	ax:sword
assume	cx:sword

	call	SetFastDot
	call	DoDot

	;Get the change in y coordinates
	mov	yVector, VROWBYTES
	mov	ax, endY
	sub	ax, penY
	.if	sign?
		neg	yVector
		neg	ax
	.endif
	mov	yDelta, ax

	;Get the change in x coordinates and its sign 
	mov	xVector, 1
	mov	ax, endX
	sub	ax, penX
	.if	sign?
		;negative motion, complement vector and delta
		neg	xVector
		neg	ax
	.endif
	mov	xDelta, ax

	;Branch depending on magnitude of the deltas
	mov	ax, yDelta
	.if	ax < xDelta
		mov	cx, xDelta		;iterations required to reach last point
		mov	dx, cx			;init rollover counter to smooth out line
		shr	dx, 1

		;Now determine which direction X is moving and branch on that.
		.if	xVector < 0
			.repeat
				add	dx, yDelta
				.if	dx >= xDelta
					;Rollover means increment Y location before plotting
					sub	dx, xDelta
					add	si, yVector
					ShiftVNib
				.endif
				dec	penX
				FastDot
				dec	cx
			.until zero?
		.else
			.repeat
				add	dx, yDelta
				.if	dx >= xDelta
					;Rollover means increment Y location before plotting
					sub	dx, xDelta
					add	si, yVector
					ShiftVNib
				.endif
				inc	penX
				FastDot
				dec	cx
			.until zero?
		.endif
	.else
		mov	cx, yDelta		;iterations required to reach last point
		mov	dx, cx			;init rollover counter to smooth out line
		shr	dx, 1

		;Now determine which direction X is moving and branch on that.
		.if	xVector < 0
			.repeat
				add	dx, xDelta
				.if	dx >= yDelta
					;rollover means increment Y location before plotting
					sub	dx, yDelta
					dec	penX
				.endif
				add	si, yVector
				ShiftVNib
				FastDot
				dec	cx
			.until zero?
		.else
			.repeat
				add	dx, xDelta
				.if	dx >= yDelta
					;rollover means increment Y location before plotting
					sub	dx, yDelta
					inc	penX
				.endif
				add	si, yVector
				ShiftVNib
				FastDot
				dec	cx
			.until zero?
		.endif
	.endif

	return

DLine	endp

assume	di:nothing



Line	proc	near
;Plot all the points along a line from AX/BX to CX/DX

	push	ds
	pop	es

	.if	mirrorX & TRUE
		MirrorX cx
		MirrorX ax
	.endif

	; offset all 4 end points
	mov	si, rThePort
assume	si:ptr RGrafPort
	add	ax, [si].origin.h
	add	cx, [si].origin.h
	add	bx, [si].origin.v
	add	dx, [si].origin.v
assume	si:nothing

	mov	penX, ax
	mov	penY, bx
	mov	endX, cx
	mov	endY, dx

	call	SetMaps

	;Optimize for strict vertical/horizontal lines
	mov	ax, endY
	.if	ax == penY
		call	HLine
	.else
		mov	ax, endX
		.if	ax == penX
			call	VLine
		.else
			call	DLine
		.endif
	.endif

	return

Line	endp


FRAMESIZE	equ	6
UP				equ	-1
DN				equ	1
DONE			equ	0

xs   			equ	0					;scan point in line
ls   			equ	1					;left end of this line
rs   			equ	2					;right end of this line
ys				equ	3					;ycoord of this line
eb				equ	4					;extra bits of xs, ys, ls, rs
cret			equ	5					;caller of this frame UP/DN/DONE
ls1			equ	ls+FRAMESIZE	;used to limit retrace
rs1			equ	rs+FRAMESIZE	;of line looking for holes
eb1			equ	eb+FRAMESIZE	;for speed purposes



Fill	proc	near
; Fill area enclosing seed point in port's pen color
; Uses stack (BP) hungry recursive (logic limited) scan technique
; seed point (penX/Y) determines background color )
; All bitmap routines assume si holds address of left most pixel on
; current scanline.  Fill is limited by boundaries contained in theRect
; During operation the following variables are used:
; 	penX	 	Current pen X in globals
; 	penY	 	Current pen Y in globasl
; 	rightEnd 	right most (+ 1) x of line just filled
; 	leftEnd 	left most pixel of line just filled
; 	xScan 		Current x pixel of search above OR below for unfilled points

	push	ds
	pop	es

	.if	mirrorX & TRUE
		MirrorX ax
	.endif

	;Offset input coords
	mov	si, rThePort
assume	si:ptr RGrafPort
	add	ax, [si].origin.h
	mov	penX, ax
	add	bx, [si].origin.v
	mov	penY, bx

	;Copy portRect to local and offset it for clipping
	lea	si, [si].portRect
	lea	di, theRect
	mov	cx, 4
	rep	movsw

	;Add the origin of rThePort at DI to rectangle at SI
	mov	di, rThePort
	lea	si, theRect

assume	di:ptr RGrafPort
assume	si:ptr RRect
	mov	ax, [di].origin.v
	add	[si].top, ax
	add	[si].bottom, ax
	mov	ax, [di].origin.h
	add	[si].left, ax
	add	[si].right, ax
assume	si:nothing
assume	di:nothing

	push		bp
	mov		bp, fillStackHandle
	les		bp, ds:[bp]
	mov		fillStackSeg, es
	add		bp, FRAMESIZE*2			; reserve room for error checking
;CC	add		bp, 200							; reserve room for padding
	mov		fillStackBase, bp

	; Point es:bp at base of the pseudo-stack
	add		bp, FILL_STACK_SIZE-(FRAMESIZE*5)	; leave room for 1st 3 frames
;CC	add		bp, FILL_STACK_SIZE-320		; leave extra room at end

	;Set up plotting mapSet
	call	SetMaps

	;Are there any maps to be plotted in?
	test	mapSet, VMAP or PMAP or CMAP
	jz		FillExit

	;Abort any attempt to fill in background color of guide map
	mov	dx, mapSet
	.if	dx & VMAP
		cmp	vColor, vWHITE
	.elseif	dx & PMAP
		cmp	pColor, 0
	.else
		cmp	cColor, 0
	.endif
	je		FillExit

assume	bp:ptr byte
assume	ax:sword
assume	bx:sword

	;If seed point is filled we abort
	SetFillBase
	mov	bx, penX
	CheckDot
	jne	FillExit

	;Bump bp down for headroom for first frame
	sub	bp, 120

	;Create a fake first frame
	mov	ax,  penX
	mov	xScan,  ax
	mov	rightSide,  ax
	mov	leftSide,  ax
	mov	oldRightSide,  ax
	mov	oldLeftSide,  ax
	mov	ax, penY
	mov	yScan, ax

	;Put DONE in the first frame of the fill stack
	mov	dl, DONE


PushFill:
;Save current parms in current frame
;This is the frame we are filling (the line)

	mov	es, fillStackSeg		; Point to pseudo-stack

	;BH accumulates spare bits
	xor	bh, bh

	;X
	mov	ax, xScan
	mov	es:[bp + xs], al
	SaveExtraBits

	;Y 
	mov	ax, yScan
	mov	es:[bp + ys], al
	SaveExtraBits

	;RS
	mov	ax, rightSide
	mov	oldRightSide, ax
	mov	es:[bp + rs], al
	SaveExtraBits

	;LS
	mov	ax, leftSide
	mov	oldLeftSide, ax
	mov	es:[bp + ls], al
	SaveExtraBits

	;BH is full of extra bits now: MSB LS/RS/YS/XS
	mov	es:[bp + eb], bh

	;Create the new frame.
	sub	bp, FRAMESIZE
	mov	es:[bp+cret], dl			;set callers direction ID

	;Routine develops local variables which are packed onto stack to recurse.
	;Must exit via a "jmp PopFill" to reinstate locals from stack.
	;On entry X org is in penX,  Y org is in penY

	SetFillBase
	mov	bx, penX
	call	CheckRight	;return right end of scan in ax
	mov	rightSide, ax
	
	mov	bx, penX

	; in 16 color version we decrement X before passing it
	; dec	bx
	call	CheckLeft	;returns left end of scan in ax
	inc	ax
	mov	leftSide, ax

	; Line ends have been found, fill line between end points
	mov	bx, leftSide
	mov	cx, rightSide
	call	FillLines

	;Now we scan above while xs < rs.
	mov	ax, leftSide
	mov	xScan, ax
	mov	ax, penY
	mov	yScan, ax


LookUp:		      
; look above this line for holes
	mov	ax, yScan
	dec	ax
	mov	penY, ax

	;If the next line up is off the top of the port, it's time to start
	;looking down.
	cmp	ax, theRect.top
	jl		LookDnStart

	SetFillBase
	.repeat
		mov	ax, xScan
		mov	penX, ax
		mov	bx, ax

		mov	es, fillStackSeg
		.while TRUE
			cmp	ax, rightSide
			jge	LookDnStart
			inc	xScan

			.break .if es:[bp+cret] != DN || ax >= oldRightSide || ax <= oldLeftSide

			;We're in between so we swap them
			mov	ax, oldRightSide
			mov	xScan, ax
		.endw

		CheckDot
	.until zero?

	;Check for stack limit
	.if	bp >= fillStackBase
		mov	dl, UP
		jmp	PushFill
	.else
		invoke Panic, E_FILL
	.endif

PopFill:
;get saved locals off of the pseudo stack and return to proper caller
;via a jump

	mov	cx, rightSide			;return to caller
	mov	es, fillStackSeg
	mov	dl, es:[bp+cret]		;caller's id
	add	bp, FRAMESIZE			;adjust frame up

	;Unpack stack frame.  Extra bits are MSB LS/RS/YS/XS
	mov	bh, es:[bp + eb]

	;L
	mov	al, es:[bp + ls]
	GetExtraBits
	mov	leftSide, ax

	;R
	mov	al, es:[bp + rs]
	GetExtraBits
	mov	rightSide, ax

	;Y
	mov	al, es:[bp + ys]
	GetExtraBits
	mov	yScan, ax

	;X
	mov	al, es:[bp + xs]
	GetExtraBits
	mov	xScan, ax

	;Now get oldRight/LeftSide from frame above into locals
	mov	bh, es:[bp + eb1]

	;L
	mov	al, es:[bp + ls1]
	GetExtraBits
	mov	oldLeftSide, ax

	;R
	mov	al, es:[bp + rs1]
	GetExtraBits
	mov	oldRightSide, ax

	;All unpacked.
	mov	ax, cx		;rightSide from completed recursion

	.if	dl == DN
		mov	xScan, ax
		jmp	LookDn
	.endif

	.if	dl == UP
		mov	xScan, ax
		jmp	LookUp
	.endif



FillExit:
;*******THIS IS THE MAIN EXIT********

	pop		bp
	push		ds
	pop		es
	ret


LookDnStart:
; scan to left on bottom of line
	mov	ax, leftSide
	mov	xScan, ax

LookDn:
	mov	ax, yScan
	inc	ax
	mov	penY, ax

	;If the next line down is off the bottom of the port, it's time to start
	;returning from the recursion.
	cmp	ax, theRect.bottom
	jge	PopFill

	SetFillBase
	.repeat
		mov	ax, xScan
		mov	penX, ax
		mov	bx, ax

		mov	es, fillStackSeg
		.while TRUE
			cmp	ax, rightSide
			jge	PopFill

			inc	xScan

			.break .if es:[bp+cret] != UP || ax >= oldRightSide || ax <= oldLeftSide

			mov	ax, oldRightSide
			mov	xScan, ax
		.endw

		CheckDot
	.until zero?

	;open hole set up to recurse
	; check for stack limit
	cmp	bp, fillStackBase
	jnb	skipOverPanic
		invoke Panic, E_FILL
		jmp		PopFill
skipOverPanic:
	mov	dl, DN
	jmp	PushFill



CheckRight:
; scans right from penX/Y returning X of first collision in AX
; SI has address of base of line in question, BX is pixel from left

	;Scan full bytes for deviation
	mov	cx, theRect.right
	sub	cx, bx
	mov	dx, cx

	; address first pixel in question
	lea	di, [si][bx]

	.if	mapSet & VMAP
		mov	es, vSeg
		mov	al, vWHITE
		.repeat
			; comparing to full bytes of color bitmap
			.break .if al != es:[di]
			inc	di
		.untilcxz
	.else
		mov	es, pcSeg							; guide seg is PMAP or CMAP
		.if	mapSet & PMAP
			mov	al, EVENON							; default to priority nibble
		.else
			mov	al, ODDON
		.endif
		.repeat
			.break .if es:[di] & al				; test appropriate nibble (in AL)
			inc	di
		.untilcxz
	.endif

	; All done.  Return new penX in AX.
	; (difference between dx and cx is pixels we advanced)
	sub	dx, cx
	mov	ax, bx
	add	ax, dx
	ret


CheckLeft:
; scans left from penX/Y returning X of first collision in AX
; SI has address of base of line in question

	;Scan full bytes for deviation
	; this code needs to be modified for less than full screen ports
	mov	cx, bx	
	sub	cx, theRect.left
	inc	cx
	mov	dx, cx

	; address first pixel in question
	lea	di, [si][bx]

	.if	mapSet & VMAP
		mov	es, vSeg
		mov	al, vWHITE
		.repeat
			; comparing to full bytes of color bitmap
			.break .if al != es:[di]
			dec	di
		.untilcxz
	.else
		mov	es, pcSeg							; guide seg is PMAP or CMAP
		.if	mapSet & PMAP
			mov	al, EVENON						; default to priority nibble
		.else
			mov	al, ODDON						; pixels in even nibble
		.endif
		.repeat
			.break .if es:[di] & al				; test appropriate nibble (in AL)
			dec	di
		.untilcxz
	.endif

	; All done.  Return new penX in AX.
	; (difference between dx and cx is pixels we moved to left)
	sub	dx, cx
	mov	ax, bx
	sub	ax, dx
	; if we made it to an edge without a hit we must
	; fudge and decrement AX by one
	;or	cx, cx
	;.if	equal
	;	inc	ax
	;.endif
	ret

assume	bp:nothing
assume	ax:nothing
assume	bx:nothing

Fill	endp



FillLines	proc	near uses es bp
; Fill an entire line in all specified pens
;Using:	SI = Base offset of line, BX = left end, CX = right end
;	ES:DI for stosb, BP has mapSet, DX holds run count

	mov	bp, mapSet
	lea	di, [si][bx]
	sub	cx, bx
	.if	!zero?
		mov	dx, cx
		.if	bp & VMAP
			mov	al, vColor
			mov	es, vSeg
			rep	stosb
			mov	cx, dx							; reload run and base address
			lea	di, [si][bx]
		.endif

		; mask off visual map bit and
		; try for a straight store to shared priority/control map
		and	bp, PMAP or CMAP
		.if	bp == (PMAP or CMAP)
			mov	al, pColor
			or		al, cColor
			mov	es, pcSeg
			rep	stosb
		.else
			.if	bp & (PMAP or CMAP)
				; set up for one or the other
				mov	es, pcSeg
				.if	bp & PMAP
					mov	al, pColor
					mov	ah, EVENOFF
				.else
					mov	al, cColor
					mov	ah, ODDOFF
				.endif
				.repeat
					and	es:[di], ah
					or		es:[di], al
					inc	di
				.untilcxz
			.endif
		.endif
	.endif

	ret

FillLines	endp



SetMaps	proc	near
;Set up local colors into nibbles and set mapSet for active bitmaps

	mov	mapSet, 0

	.if	vColor != PENOFF
		or		mapSet, VMAP
	.endif

	.if	pColor != PENOFF
		or		mapSet, PMAP
	.endif

	.if	cColor != PENOFF
		or		mapSet, CMAP
	.endif

	ret

SetMaps	endp



SizeRect proc uses si bx, rPtr:ptr RRect
; modify rectangle to even pixel size
; return storage size in bytes of enclosed pixels

	mov	si, rPtr
assume	si:ptr RRect
	mov	bx, [si].bottom
	sub	bx, [si].top
	mov	ax, [si].right
	sub	ax, [si].left
assume	si:nothing
	mul	bx

	ret

SizeRect endp


InitGraph proc uses di
LOCAL mapSize:word
;Allocate virtual bitmaps and resolution specific initialization
	
	push	ds
	pop	es

	; create base address table
	mov	ax, 0
	lea	di, baseTable
	mov	cx, bounds.bottom
	sub	cx, bounds.top
	.repeat
		stosw
		add	ax, VROWBYTES
	.untilcxz

	; allocate memory for visual & shared priority/control virtual bitmaps
	invoke	SizeRect, addr bounds
	mov	mapSize, ax

	; get a handle (and dereference it) for each map in turn
	invoke	NeedHandle, ax
	mov	bx, ax
	mov	vHndl, ax
	mov	ax, [bx+_seg]
	mov	vSeg, ax 
	invoke	LockHandle, bx

	; get a pri/control bitmap
	invoke	NeedHandle, mapSize
	mov	bx, ax
	mov	pcHndl, ax
	mov	ax, [bx+_seg]
	mov	pcSeg, ax 
	invoke	LockHandle, bx

	; clear all vMaps to default colors
	push	es
	xor	al, al

	mov	es, vSeg
	mov	cx, mapSize
	xor	di, di
	rep 	stosb

	mov	es, pcSeg
	mov	cx, mapSize
	xor	di, di
	rep 	stosb
	pop	es

	ret

InitGraph endp



EndGraph proc
;Free allocated virtual bitmap's memory

	invoke	DisposeHandle, vHndl
	invoke	DisposeHandle, pcHndl
	ret

EndGraph endp



SaveBits proc uses si di,	rPtr:ptr RRect, lmapSet:word
; Save virtual bitmaps bounded by rect, in buffer we allocate locally.
; Return handle to buffer.

	mov	ax, ds
	mov	es, ax

	; address the rectangle
	mov	si, rPtr
	call	GetTheRect

	; clip it and offset it
	mov	di, rThePort
	lea	di, (RGrafPort ptr [di]).portRect
	lea	si, theRect
	mov	bx, si									; it's its own destination
	call	RSectRect
	.if	!sign?
		mov	rPtr, 0
		jmp	SBExit
	.endif
	
	mov	di, rThePort
	call	SOffsetRect

	; get a MEMORY resource of this many bytes
	invoke	ResLoad, RES_MEM, 14
	mov	si, ax
	mov	rPtr, ax
	.if	!ax
		invoke Panic, E_SAVEBITS
	.endif

	mov	di, rPtr
	les	di, [di]
	; Copy input rectangle (global coords) into save area
	lea	si, theRect
	mov	cx, 4
	rep	movsw

	lea	si, theRect
	invoke	SizeRect, si						; rounds pixels to bytes and returns size
	mov	cx, ax									; size to save

	.if	lmapSet & VMAP
		; get a MEMORY resource of this many bytes
		push	cx
		invoke	ResLoad, RES_MEM, cx
		pop	cx
		.if	!ax
			invoke Panic, E_SAVEBITS
		.endif
		mov	di, rPtr								; find the header again
		les	di, [di]								; the damn thing keeps jumping around!
		add	di, 8									; VMAP offset
	.else
		sub	ax, ax
	.endif
	stosw												; pointer to save area for CMAP

	.if	lmapSet & (PMAP or CMAP)
		invoke	ResLoad, RES_MEM, cx
		.if	!ax
			invoke Panic, E_SAVEBITS
		.endif
		mov	di, [rPtr]							; find the header again
		les	di, [di]								; the damn thing keeps jumping around!
		add	di, 10								; PMAP/CMAP offset
	.else
		sub	ax, ax
	.endif
	stosw												; pointer to save area for PMAP/CMAP
	mov	word ptr es:[di],4321h				; So we can tell this is a savebits
														; memory area

	; set up registers and vars for saving one bitmaps rectangles
	mov	dx, theRect.bottom
	mov	si, theRect.top
	sub	dx, si
	mov	vRun, dx
	Base	si, baseTable

	mov	bx, theRect.right
	mov	cx, theRect.left
	sub	bx, cx
	;mov	hRun, bx
	add	si, cx

	; AX is inter-line advance (difference in width and runcount)
	mov	ax, VROWBYTES
	sub	ax, bx

	push	ds

	; save a bitmap
	mov	di, es:[di-4]
	.if	di
		les	di, [di]								; point to VMAP save area
		push	si
		mov	dx, vRun								; set vertical run
		mov	ds, vSeg
		.repeat
			mov	cx, bx
			rep	movsb
			add	si, ax
			dec	dx
		.until zero?
		pop	si
	.endif

	mov	di, ss:[rPtr]
	les	di, ss:[di]
	mov	di, es:[di+10]
	.if	di
		les	di, ss:[di]							; point to PMAP/CMAP save area
		mov	dx, ss:vRun							; set vertical run
		mov	ds, ss:pcSeg
		.repeat
			mov	cx, bx
			rep	movsb
			add	si, ax
			dec	dx
		.until zero?
	.endif

	pop	ds

SBExit:
	mov	ax, rPtr
	ret

SaveBits endp



RestoreBits proc uses si di, hndl:word
; Restore virtual bitmaps with info and data from passed handle.
; Dispose of handle.   Return nothing. 
; rectangle in data is in Global coords and should stay that way

	; de-reference the handle to examine save area header
	mov	si, hndl
	or		si, si
	jz		RBExit

	; address the save area
	push	es
	push	si											; save header
	les	si, [si]

	; check validity of the handle
	.if	word ptr es:[si+12] != 4321h
		pop	si
		pop	es
		jmp	RBExit
	.endif
	mov	word ptr es:[si+12],0000h

	; rectangle is first 4 words of SaveArea
	; set vertical run count
assume	si:ptr RRect
	mov	di, es:[si].top
	mov	dx, es:[si].bottom
	sub	dx, di
	mov	vRun, dx
	Base	di, baseTable

	; set horizontal run count
	mov	bx, es:[si].right
	mov	cx, es:[si].left
	sub	bx, cx
	;mov	hRun, bx
	add	di, cx
assume	si:nothing

	; AX is inter-line advance (difference in width and runcount)
	mov	ax, VROWBYTES
	sub	ax, bx

	mov	si, es:[si+8]		; VMAP save area
	.if	si
		push	di
		mov	dx, vRun								; set vertical run
		mov	es, vSeg

		push	ds
		push	si
		lds	si, [si]								; pointer to VMAP bits
		.repeat
			mov	cx, bx
			rep	movsb
			add	di, ax
			dec	dx
		.until zero?
		pop	si
		pop	ds
		pop	di
		
		push	ax										; save inter-line advance
		push	bx										; save hRun
		invoke	ResUnLoad, RES_MEM, si
		pop	bx
		pop	ax
	.endif

	pop	si

	les	si, [si]
	mov	si, es:[si+10]							; PMAP/CMAP save area
	.if	si
		push	di
		mov	dx, vRun								; set vertical run
		mov	es, pcSeg

		push	ds
		push	si
		lds	si, [si]								; pointer to VMAP bits
		.repeat
			mov	cx, bx
			rep	movsb
			add	di, ax
			dec	dx
		.until zero?
		pop	si
		pop	ds
		pop	di
		
		invoke	ResUnLoad, RES_MEM, si
	.endif

	; all done.  Dispose of the handle we got
	pop	es
	invoke	ResUnLoad, RES_MEM, hndl

RBExit:
	ret

RestoreBits endp



UnloadBits proc	hndl:word
;Dispose of handle.   Return nothing. 

	; de-reference the handle to examine save area header
	mov	bx, hndl
	or		bx, bx
	je		UBExit

	; address the save area
	push	es
	push	bx											; save header
	les	bx, [bx]

	; check validity of the handle
	.if	word ptr es:[bx+12] != 4321h
		pop	bx
		jmp	UBExit								;not underbits, just unload
	.endif
	mov	word ptr es:[bx+12],0000h

	mov	bx, es:[bx+8]							; VMAP save area
	.if	bx
		invoke	ResUnLoad, RES_MEM, bx
	.endif

	pop	bx

	les	bx, [bx]
	mov	bx, es:[bx+10]							; PMAP/CMAP save area
	.if	bx
		invoke	ResUnLoad, RES_MEM, bx
	.endif

	pop	es
	invoke	ResUnLoad, RES_MEM, hndl

UBExit:
	ret

UnloadBits endp



GetSegment proc
;Return segment of a virtual bitmap, bit indicated by AX from rThePort

	.if	ax & (CMAP or PMAP)
		mov	ax, pcSeg
	.else
		mov	ax, vSeg								; return vSeg by default
	.endif
	ret
 
GetSegment endp



ShowBits proc uses si di,	rPtr:ptr RRect, lmapSet:word
;Provide binding to video device driver

	push	ds
	pop	es

	; clip it and offset input rectangle
	mov	si, rPtr
	mov	di, rThePort
	lea	di, (RGrafPort ptr [di]).portRect
	lea	bx, showRect							; its destination
	call	RSectRect
	jge	SBExit									; no rect left to show
	mov	di, rThePort
	mov	si, bx
	call	SOffsetRect								; SI still valid

	; get segment of requested bitmap
	mov	ax, lmapSet
	call	GetSegment
	mov	si, ax

	; get the rectangle boundaries into registers
	lea	di, showRect
assume	di:ptr RRect
	mov	ax, [di].top
	mov	bx, [di].left
	mov	cx, [di].bottom
	mov	dx, [di].right
assume	di:ptr RRect
	mov	di, lmapSet
	mov	bp, V_SHOWBITS
	call	DWORD PTR Display

SBExit:
	ret

assume	di:nothing

ShowBits endp


SetOutputPage proc near uses bp, outputPage:word
	mov	ax, outputPage
	mov	bp, V_SETOUTPAGE
	call	DWORD PTR Display
	ret

SetOutputPage endp

GetOutputPage proc near uses bp
	mov	bp, V_GETOUTPAGE
	call	DWORD PTR Display
	ret
GetOutputPage endp


ClearPagedScreens proc uses bp
	mov	bp, V_CLEARPAGEMODE
	call	Display
	ret
ClearPagedScreens endp


KGraph	proc	uses si, args:word
;Common dispatch for various kernel routines

	mov	si, args
	lodsw
	mov	cx, ax
	lodsw

	.if		 ax == GLoadBits
		call	LoadBits

	.elseif ax == GDetect
		mov	bp, D_DETECT
		call	Display
		mov	acc, ax
     mov   NumberColors, ax

	.elseif ax == GDrawLine
		; y1, x1, y2, x2, v, p, c
		mov	ax, [si + 8]
		mov	vColor, al
		mov	ax, [si + 10]
		.if	ax != -1
			shl	al, 1
			shl	al, 1
			shl	al, 1
			shl	al, 1
		.endif
		mov	pColor, al
		mov	ax, [si + 12]
		mov	cColor, al
		mov	ax, [si + 2]
		mov	bx, [si]
		mov	cx, [si + 6]
		mov	dx, [si + 4]
		call	Line

	.elseif ax == GReAnimate
		invoke	ReAnimate, si

	.elseif ax == GSaveBits
		invoke	SaveBits, si, [si+8]
		mov	acc, ax

	.elseif ax == GRestoreBits
		invoke	RestoreBits, [si]

	.elseif ax == GEraseRect
		invoke	REraseRect, si

	.elseif ax == GPaintRect
		invoke	RPaintRect, si

	.elseif ax == GFillRect
		invoke	RFillRect, si, [si+8], [si+10], [si+12], [si+14]

	.elseif ax == GShowBits
		invoke	ShowBits, si, [si+8]

	.endif

	ret 

KGraph endp



RHideCursor proc uses si
;Tell driver to hide the cursor

	push	bp
	mov	bp, V_HIDECUR
	call	Display
	pop	bp
	ret

RHideCursor endp



RShowCursor proc
; tell driver to show cursor on screen

	push	bp
	mov	bp, V_SHOWCUR
	call	Display
	pop	bp
	ret

RShowCursor endp



LoadBits	proc	near
;load the numbered bitmap at SI

	ret

LoadBits	endp



StdChar proc uses si di
LOCAL dimMask:byte, cWide:byte, cHigh:byte, penMod:word, style:word, \
		theChar:word, theFontSeg:word
; draw the character in AX
; set starting address of this first byte

	push	es
	mov	theChar, ax

	; prepare color masks in DL & DH
	mov	si, rThePort
assume	si:ptr RGrafPort
	mov	dx, [si].fgColor

	; set character face
	mov	bx, [si].txFace
	mov	style, bx
	mov	di, [si].pnLoc.v
	add	di, [si].origin.v
	mov	penY, di
	Base	di, baseTable
	mov	bx, [si].pnLoc.h
	add	bx, [si].origin.h
	mov	penX, bx
	add	di, bx

	; we address the character patterns here
	; ******** don't disturb DI or DX ********
	push	dx
	invoke	ResLoad, RES_FONT, [si].fontNum
	pop	dx
	mov	si, ax
assume	si:nothing
	mov	ax, [si+_seg]
	mov	theFontSeg, ax
	mov	si, [si+_off]
assume	si:ptr Font

	; prepare the character variables
	mov	es, theFontSeg
	mov	bx, theChar								; character's code
	.if	bx < es:[si].highChar && bx >= es:[si].lowChar
		; get the character width and height and address patterns
		shl	bx, 1
		add	si, es: [si+bx].charRecs
assume	si:nothing
		mov	al, es:[si]
		cbw
		mov	hRun, ax
		inc	si
		mov	cWide, al
		mov	al, es:[si]
		cbw
		mov	vRun, ax
		inc	si
		mov	cHigh, al

		; SI points at first byte of patterns

		; set byte and mod for later
		mov	bx, penX
		and	bx, 1
		mov	penMod, bx
		mov	bx, penX
;		mov	penByte, bx

		; DH and DL have color
		.repeat
			; draw a complete line of the character
			mov	al, 0ffh
			.if	style & 1
				and	al, 055h
				.if	penY & 1
					not	al
				.endif
			.endif
			mov	dimMask, al

			push	di
			mov	ax, penMod						; load our floating dot mask
			mov	bl, 0								; BL is current dot in character

			; get next byte of pattern
			mov	es, theFontSeg
			mov	ah, es: [si]
			inc	si
			and	ah, dimMask						; ff if not dim at all
			mov	bh, cWide

			.while TRUE
				;Draw a byte of the character's pattern

				mov	es, vSeg
				shl	ah, 1							;we shift dots out the left and plot left to right
				.if	carry?
					mov	es:[di], dl
				.endif
				inc	al
				inc	di
				inc	bl
				.break .if bl >= bh				;width of dots

				; check for byte reload
				.if	!(bl & 7)					; 8 bits used?
					; load AH with a byte of pattern and inc si
					mov	es, theFontSeg
					mov	ah, es: [si]
					and	ah, dimMask				; ff if not dim at all
					inc	si
				.endif
			.endw

			; line of pattern done
			pop	di
			add	di, VROWBYTES

			; see if we have more lines to do
			inc	penY
			dec	cHigh
		.until zero?
	.endif

	pop	es
	ret

StdChar	endp



OnControl proc uses  si di, map:word, rPtr:ptr RRect
;Return bit significant word of all controls this rectangle encompasses
	
	push	ds
	pop	es

	; get rect into local
	mov	si, rPtr
	lea	di, theRect
	mov	cx, 4
	rep	movsw

	mov	di, rThePort
	lea	di, (RGrafPort ptr [di]).portRect
	lea	si, theRect
	mov	bx, si									; it's its own destination
	call	RSectRect
	mov	ax, 0										; default 0 return if no rect left
	jge	OCExit									; no rectangle left to check

	; make it global
	mov	di, rThePort
	lea	si, theRect
	call	SOffsetRect

	; address the first byte 
	; set run count vars
	; calculate interline advance value
	mov	di, theRect.top
	mov	ax, theRect.bottom
	sub	ax, di
	mov	vRun, ax
	Base	di, baseTable
	mov	bx, theRect.left
	mov	cx, theRect.right
	add	di, bx
	sub	cx, bx
	mov	hRun, cx
	mov	si, map
	mov	bp, VROWBYTES
	sub	bp, cx
	
	; do all the lines
	push	es
	mov	es, pcSeg
	xor	bh, bh
	mov	ax, 0										; accumulates bits
	.repeat
		.if	si & PMAP
			.repeat
				; priority pixel is in high nibble
				mov	bl, es:[di]
				inc	di
				and	bl, EVENON
				shr	bl, 1
				shr	bl, 1
				shr	bl, 1
				or		ax, wordBits[bx]
			.untilcxz
		.else
			.repeat
				; control pixel is in low nibble
				mov	bl, es:[di]
				inc	di
				and	bl, ODDON
				shl	bl, 1
				or		ax, wordBits[bx]
			.untilcxz
		.endif
		add	di, bp								; advance DI to next line
		mov	cx, hRun								; reload runcount
		dec	vRun									; decrement vertical run count
	.until zero?

	pop	es

OCExit:
	ret

OnControl endp



; VIDEO DEVICE DRIVER BINDINGS


SetCLUT	proc	uses si,	palOff:word, palSeg:word, palClut:word

	; arrange parms to call driver
	push	bx
	mov	ax, palSeg
	mov	si, palOff

	mov	bx, V_SETPALETTE
	cmp	palClut, 0
	je	   noPalCycle
	mov	bx, V_SETPALETTE_CYCLE

noPalCycle:
	mov	bp, bx
	call	Display
	pop	bx
	ret

SetCLUT	endp



GetCLUT	proc	uses si, palOff:word, palSeg:word

	mov	ax, palSeg
	mov	si, palOff
	mov	bp, V_GETPALETTE
	call	Display
	ret

GetCLUT	endp



InitDisplay proc
;Set display mode and return old mode

	push	bp
	mov	bp, D_INIT
	mov	ax, offset sysTicks
	mov	dx, ds
	call	Display
	pop	bp
	ret

InitDisplay endp



EndDisplay proc	vMode:word
; return display to passed mode

	push	bp
	mov	ax, vMode
	mov	bp, D_TERMINATE
	call	Display
	pop	bp
	ret

EndDisplay endp



MoveCursor proc	theX:word, theY:word
; tell driver to move cursor to new position

	mov	ax, theX
	mov	bx, theY
	mov	bp, V_MOVECUR
	call	Display
;	sti
	ret

MoveCursor endp


;  !!!
PointToCel proc uses ds, theView:word, theLoop:word, theCel:word
; returns ax:si points to cel
	mov	si, theView
	mov	ax, [si+_seg]
	push	ax
	mov	si, [si+_off]
	push	si
	mov	ds, ax
	xor	bh, bh
	mov	bx, theLoop
assume si: ptr View
	mov	cl, [si].numLoops
	.if	bl >= cl
		mov	bl, cl
		dec	bl
	.endif

	mov	al, [si].loopHeaderSize
	mul	bl
	add	ax, [si].viewHeaderSize	; ax now has offset of the loop header
	add	ax, 2							; because view header size not included in above
	mov	bl, [si].celHeaderSize
	add	si, ax						; now pointing to the loop header
assume si: ptr Loop	
	
	mov	ax, theCel
	.if	al > [si].numCels
		mov	al, [si].numCels
		dec	al
	.endif

	mul	bl									; ax = celheader size * cel no
	add	ax, word ptr [si].celOffset	; ax = pointer to desired cel header
	mov	si, ax
	pop	bx		; the view offset
	pop	ax
	add	si, bx
	

assume si: nothing

	ret			; ax = view segment, si = cel header offset

PointToCel endp



Expand proc
;input:
;       es:di points to destination for cursor data
;       ds:si points to run length control cursor data
;       ds:bx points to cursor color data
;       cx    contains bytes of cursor data
;       ah    contains skip color
;

	.while cx										

		lodsb                					;al = dbyte
		xor	dh, dh		
		mov	dl, al         					;dl = run
		and	dl, 03fh
		sub	cx, dx 
		.break .if sign?

		.if	al & REPEATC
			;repeat same byte
			.if	al & REPSKIP
				mov	al, ah						; the skip color
			.else
				mov	al, [bx]						; get the color byte
				inc	bx								; point to next color
			.endif
			.repeat									; store the skip or repeated color
				stosb
				dec	dl
			.until zero?
		.else
			;unique bytes
			.repeat
				mov	al, [bx]						; get the color 
				inc	bx
				stosb
				dec	dl
			.until zero?
		.endif
	.endw

	ret

Expand endp



RSetCursor proc uses si di, theView:word, theLoop:word, theCel:word,	hotX:word, hotY:word
; give driver a new cursor definition

	push	ds											; Save ds for call to showcur
	invoke	PointToCel, theView, theLoop, theCel 
	; returns si (offset) ax (seg) pointer to cel

	push	ax
	push	si
	push	bp
	mov	bp, V_SETCUR
	call	Display									; point to cursor area (return in dx:ax)
	mov	es, dx
	mov	di, ax
	pop	bp
	pop	si
	pop	ds

	; move xDim,yDim,xHot,yHot and skip color
	push	si											; pointer to cel
	lodsw												;xDim
	mov	bx, ax
	stosw
	mul	word ptr [si]
	mov	cx, ax									;xDim*yDim
	.while cx > MAXCURSORBUF
		sub	cx, bx
		dec	word ptr [si]
	.endw
	lodsw												;yDim
	mov	dx, ax
	stosw

	; If hotX and hotY not passed, calculate xHot and yHot
	; from the view's x and y origin
	.if	hotX == -1
		lodsw				   						;xOrg
		neg	ax
		shr	bx, 1		   						;xDim/2
		add	ax, bx
		stosw				   						;xHot
		lodsw				   						;yOrg
		neg	ax
		add	ax, dx
		dec	ax
		stosw											;yHot
	.else
		;ignore xOrg and yOrg
		inc	si
		inc	si
		inc	si
		inc	si
		mov	ax, hotX								;hotX
		stosw
		mov	ax, hotY								;hotY
		stosw
	.endif

	lodsb				   							;skip color
	mov	dl, al
	xor	ah, ah
	stosw

	pop	si											; get back pointer to cel
assume si:ptr Cel
	mov	bx, word ptr [si].colorOffset
	mov	si, word ptr [si].dataOffset

	push	di			;offsets are relative to view
	mov	di, theView
	mov	di, ss:[di+_off]
	add	bx, di
	add	si, di
	pop	di

assume si:nothing

	; decompress data into cursor
	mov	ah, dl									;skip color in ah
	invoke	Expand

	pop	ds											; Restore ds so Display points to the right place
	mov	bp, V_SHOWCUR							;show new cursor
	call	Display

	ret

RSetCursor endp



ShakeScreen proc	cnt:word, dir:word
; shake the screen

	push	bp
	mov	cx, cnt
	mov	dx, dir
	mov	ax, ds
	lea	bx, sysTicks
	mov	bp, V_SHAKE
	call	Display
	pop	bp
	ret

ShakeScreen endp



ShiftScreen	proc	uses si di, rtop:word, rleft:word, rbot:word, rright:word, dir:word
; shift the screen area defined by the passed rectangle coordinates
; in the specified direction

	.if currentVideoMode	== 2
		
		; before we can scroll, we need to copy the VMAP into the correct
		; alternate video page

		call GetOutputPage	; get the current output page in ax
		push	ax

		.if dir == 1	; scrolling right
			mov	ax, PAGE2
			push	ax								; to be used as the copy back source
			invoke SetOutputPage, PAGE2
		.elseif dir == 0	;scrolling left
			mov	ax, PAGE0					
			push	ax								; to be used as the copy back source
			invoke SetOutputPage, PAGE0
		.else
			pop	ax			; can't scroll vertically in modeXH
			ret
		.endif

		; draw the whole thing (VMAP) into the alt page
      invoke ShowBits, addr wholeScreen, VMAP	

		pop	si			; the source page for an eventual copy back
		pop	di			; the original page (now to be the destination)

		invoke SetOutputPage, di	; put the driver pointer back the way it was
											; Note: Does not change hardware

		mov	dx, dir
		mov	bp, V_HWSHIFT
		mov	ax, ds
		lea	bx, sysTicks
		call	Display					; Do the actual scroll!

		
		; now, copy the screen data back to the original page and 
		; display the original page

		mov	bp, V_COPYPAGE		; si & di are still pointing correctly
		call	Display

		mov	bx, di
		mov	BP, V_DISPLAYPAGE		; now actually display the original page
		call	Display					; including setting the hardware and pointer
		
		ret

	.else
		mov	ax, rtop
		mov	bx, rleft
		mov	cx, rbot
		mov	dx, rright
		mov	si, dir
		mov	di, vSeg
		mov	bp, V_SHIFT
	.endif

	call	Display
	ret

ShiftScreen	endp



ColorFade proc
;dummy routine for fade in/out (not available on IBM)

	ret

ColorFade endp



FastMatch	proc	uses si di, palOff:word, palSeg:word, redV: word, greenV:word,blueV:word, psize:word, least:word
LOCAL leastIndex:word
; return entry index of palette that is closest to passed R/G/B values
; match is determined by least sum of absolute differences

	; default to NO match
	mov	ax, psize
	mov	leastIndex, ax
	mov	di, least

	; init some locals
	mov	cx, 0

	; move arguments into locals
	mov	ah, byte ptr redV
	mov	bl, byte ptr greenV
	mov	bh, byte ptr blueV

	; address the palette
	mov	si, palOff
	add	si, 104h		; skip over mapTo array and valid
	mov	es, palSeg

	.repeat
		; only match active entries
		mov	al, es:[si]
		.if	al & 1
			mov	al, es:[si + 1]
			sub	al, ah
			.if	sign?
				neg	al
			.endif
			mov	dl, al
			xor	dh, dh
		
			; need we examine green?
			.if	dx <= di
				mov	al, es:[si + 2]
				sub	al, bl
				.if	sign?
					neg	al
				.endif
				add	dl, al
				adc	dh, 0

				; need we examine blue?
				.if	dx <= di
					mov	al, es:[si + 3]
					sub	al, bh
					.if	sign?
						neg	al
					.endif
					add	dl, al
					adc	dh, 0

					; sum of differences is in dx
					; our current index will be closest match
					; if dx is less than current least
					; compare to current least
					.if	dx <= di
						; update least and least index
						mov	di, dx
						mov	leastIndex, cx
					.endif
				.endif
			.endif
		.endif

		; end of loop test
		add	si, 4
		inc	cx
	.until cx == psize

	; return index of least
	; a value equal to size of palette
	; indicates NO match
	mov	ax, leastIndex
	ret

FastMatch	endp



PaletteShell	proc	uses di, palOff:word, palSeg:word

	mov	di, palOff
	add	di, 104h		; skip over mapTo array
	mov	es, palSeg
	mov   al, 1       ; in use flag
	stosb
  xor   al, al      ; set up black
  stosb
  stosb
  stosb
	mov   al, 1       ; in use flag
	mov   cx, 254
	.repeat
		stosb
		inc   di
		inc   di
		inc   di
	.untilcxz
	stosb
  mov   al, 0ffh    ; set up white
	stosb
	stosb
	stosb

	mov   ax, 100     ; intensity
	mov   cx, 256
	rep   stosw
	ret

PaletteShell	endp




PaletteServer	proc
  .if !palVaryPause
     add   palServerSignal, 1
  .endif
  ret

PaletteServer	endp



KillPalServer	proc
;disable palette server

	lea	bx, PaletteServer
	invoke DisposeServer, cs::bx
	mov   palServerSignal, 0
	ret

KillPalServer	endp


PaletteUpdate	proc uses si di, changeCLUT:word, NumberTimes:word 

	local nextPalette: word

	mov   bx, newPalette
	mov	si, [bx]
	mov   bx, [bx+2]
	mov   nextPalette, bx
	mov	es, bx
	invoke PaletteShell, si, es

	; determine the new palette percent
	mov   ax, paletteDir
  ; if there has been more than one timer interrupt speed
  ; up change to get to the target at the proper time
   imul  NumberTimes
	mov   bx, paletteStop
	add   palettePercent, ax
	.if SWORD PTR  ax <= 0
		.if SWORD PTR  palettePercent < bx  && NumberTimes
			mov   palettePercent, bx
		.endif
	.else
		.if   palettePercent > bx
			mov   palettePercent, bx
		.endif
	.endif


	push  ds
	push  es
	; set up for new palette computation
	mov   bx, palettePercent
	mov   di, startPalette
	mov   ax, [di+2]
	mov   es, ax
	mov   di, targetPalette
	mov   ax, [di+2]
	mov   ds, ax

	mov   di, 260        ; skip mapTo and valid long

	mov   cx, 256

	; compute new palette
	.repeat
		push  cx
		inc   di             ;flag byte
		xor   ah, ah
		xor   ch, ch
		mov   al, [di]       ;red byte target
		mov   cl, es:[di]    ;red byte start
		sub   ax, cx
		imul  bx
		sar   ax, 1          ;divide by 64
		sar   ax, 1
		sar   ax, 1
		sar   ax, 1
		sar   ax, 1
		sar   ax, 1
		add   al, es:[di]    ;al contains new red byte
		push  es
		mov   cx, nextPalette
		mov   es, cx
		stosb                ;red byte new current
		pop   es


		xor   ah, ah
		xor   ch, ch
		mov   al, [di]       ;blue byte target
		mov   cl, es:[di]    ;blue byte start
		sub   ax, cx
		imul  bx
		sar   ax, 1          ;divide by 64
		sar   ax, 1
		sar   ax, 1
		sar   ax, 1
		sar   ax, 1
		sar   ax, 1
		add   al, es:[di]    ;al contains new red byte
		push  es
		mov   cx, nextPalette
		mov   es, cx
		stosb                ;blue byte new current
		pop   es


		xor   ah, ah
		xor   ch, ch
		mov   al, [di]       ;green byte target
		mov   cl, es:[di]    ;green byte start
		sub   ax, cx
		imul  bx
		sar   ax, 1          ;divide by 64
		sar   ax, 1
		sar   ax, 1
		sar   ax, 1
		sar   ax, 1
		sar   ax, 1
		add   al, es:[di]    ;al contains new red byte
		push  es
		mov   cx, nextPalette
		mov   es, cx
		stosb                ;green byte new current
		pop   es
		pop   cx
	.untilcxz

	pop   es
	pop   ds

	; now see if the palette has changed
	; get current palette
	push  ds
	push  es

	lea   si, sysPalette
	add   si, 260        ; skip mapTo and valid long
	mov   ax, nextPalette
	mov   es, ax
	mov   di, 260        ; skip mapTo and valid long
	mov   cx, 512
	repe  cmpsw
	pop   es
	pop   ds
	.if	!zero?
		; palette has changed
		; update sysPalette
		lea   di, sysPalette
		add   di, 260        ; skip mapTo and valid long
		push  ds
		pop   es
		mov   ax, nextPalette
		mov   ds, ax
		mov   si, 260        ; skip mapTo and valid long
		mov   cx, 512
		rep   movsw          ;copy nextPalette to sysPalette
		push  es
		pop   ax
		mov   ds, ax
	   .if   changeCLUT &&   !picNotValid		; 
		   invoke SetCLUT, addr sysPalette, ds, TRUE
        ; if EGA colors do a showBits on entire screen
        .if NumberColors == 16
           invoke SetCLUT, addr sysPalette, ds, FALSE
           invoke ShowBits, addr bounds, VMAP
	      .endif
	   .endif
	.endif

	; Are we back to the original palette?
	mov   ax, palettePercent
	.if   ax
		.if   ax == paletteStop
			call	KillPalServer
		.endif
	.else
		call	KillPalServer
		invoke	ResUnLoad, RES_PAL, paletteRes
		invoke	ResUnLoad, RES_MEM, startPalette
		invoke	ResUnLoad, RES_MEM, newPalette
		mov   palVaryOn, 0
	.endif

	ret

PaletteUpdate	endp
	


PaletteCheck	proc

	xor   ax, ax
	xchg  ax, palServerSignal
	.if	ax
	  invoke	PaletteUpdate, DOSETCLUT, ax
	.endif
	ret

PaletteCheck	endp

DoShortBrush	proc
	jmp	DispatchGrf

DoShortBrush	endp

SetBrSize		proc
	jmp	DispatchGrf
SetBrSize		endp

DoAbsBrush		proc
	jmp	DispatchGrf
DoAbsBrush		endp

DoMedBrush		proc
	jmp	DispatchGrf
DoMedBrush		endp

OldSetPalette proc
	jmp	DispatchGrf
OldSetPalette endp





	
	end



