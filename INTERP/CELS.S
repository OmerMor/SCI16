;-------------------------------------------------------------------
;	cels256.s
;
; Modified 4/92 by Ken Koch
;
; This module contains all routines for outputing cels in the NEW (4/92) 
; file format to the virtual screen.  
;
; The entry point for drawing Picture bitmaps is now DrawPicCel which
; in turn calls DrawCelZ
;
; The entry point for scaled views is ScaleDrawCel while the entry point
; for normal views is DrawCel
;
; All code for outputing data in the 16 color format has now been removed!
;
; All cels are now drawn on a line by line basis.  First they are decoded
; from the RLE format to the buffer aLine (or eaten if off the screen).
; Then, they area output either to StandardLine or ScaledLine
;
; The main line decoding/drawing loop is contained in DrawCelZ for normal
; views or ScaleDraw for scaled views.
;
;
; Modified 7/29/92 by MLP
; added two calls GetCelXOff & GetCellYOff to return a cells offset values
; also changes SetCelDim to modify the offset of a cel along with the 
; dimmentions
;
;-------------------------------------------------------------------

	
include	macro.i
include	cels.i
include	graph256.i
include	palette.i
include  picture.i
include	errmsg.i
include  graph.i

.DATA

public objectOffset
objectOffset		dw		?

;celPtr			dword	?

;pointers to line buffers for drawcel
endX				sword	?
endY				sword	?
leftOffset		word	?
lineWidth		word	?
realWidth		word  ?

aLine				byte  320 dup (?)
public mirrored
mirrored			byte	0
;public noScale
;noScale			byte	0
scaled			byte	0
public decodeError
decodeError		byte	1
penX				sword	?
penY				sword	?
;hOff				sword	?
;vOff				sword	?
;remap				word	?
skip				byte	?
priMask			byte	?	; used to mask off priority ram nibble

MAXPRI			equ	010h	; maximum "normal" priority

REPEATC			equ	080h
REPSKIP			equ	040h
UNIQUE			equ	000h



.CODE

;----------------------------------------------------------------------------
; GetCelPointer
;
; This routine takes a pointer to the view pointer, the loop, and the cel
; and returns the cel segment in DX and the cel offset in AX.
;
; It also returns the view header offset in DI.
;
; It sets the global "mirrored" flag if the cel is mirrored, and adjusts
; pointers to point to the source cel.
;

GetCelPointer proc uses es si, view:word, loop:word, cel:word

	mov	mirrored, 0

viewLoop:

	; view is a handle which needs to be dereferenced
	mov	si, view
	les	si, [si]
	mov	ax, es
	or		ax, ax

	.if zero?
		xor	dx, dx
		ret
	.endif

	mov	di, si			; we need to save the view offset
assume	si:ptr View

	; validate and address the requested loop
	mov	bx, loop
	xor	ch, ch
	mov	cl, es:[si].numLoops
	.if	bx >= cx
		mov	bx, cx	
		dec	bx
	.endif	

	; calculate the loop header location based on the loopHeaderSize and
	; the loop number

	xor	ah, ah
	mov	al, es:[si].loopHeaderSize
	mul	bl		; find the loop header offset
	
	; calculate the cel header location based on the celHeaderSize and the
	; cel number

	mov	dl, es:[si].celHeaderSize

	add	ax, es:[si].viewHeaderSize
	add	ax, 2			; because the viewheadersize word not included in above
	add	si, ax		; the loop header offset

assume si:ptr Loop
	
	; check to see if the loop is mirrored or shared

	.if es:[si].altLoop != -1

		; it is either mirrored or shared
		.if es:[si].flags == 1	
			mov	mirrored, 1			; it's mirrored
		.endif

		mov	al, es:[si].altLoop	; the altLoop is the loop which is mirrored or shared
		xor	ah, ah
		mov	loop, ax
		jmp	viewLoop			; do it over again with the adjusted loop #
	.endif

	; validate the cel number
	mov	al, es:[si].numCels
	xor	ah, ah

	.if cel < ax
		mov	ax, cel
	.else
		dec	al		; offset the cel number to 0
	.endif
	
	; al now has a valid cel number 
	mul	dl		; cel# * celHeaderSize
	add	ax, word ptr es:[si].celOffset
	add	ax, di		; adjust from starting offset of view

	mov	dx, es

assume	si:nothing
	ret				; returning offset in ax, segment in dx

GetCelPointer	endp

;------------------------------------------------------------------------
; BuildLine
;
; Build a line of the cel to the buffer "aLine" from the rle data.
; This accomodates only scaled and non-mirrored non-scaled cels
; See BuildMirroredLine for non-scaled mirrored cels
;
;

BuildLine proc near uses di cx
LOCAL runX:word

; On Entry ...
; es:0 = cel 
; ds = ss
; bx points to color buffer
; si points to control buffer
; dh high nibble has priority, low nibble f


assume	si:nothing, di:nothing

	; init loops vars

	mov	ax, lineWidth			; the visual width of the cel which may (in the
										; case of scaled views) be different from the
										; cel width

	mov	runX, ax

	xor	cx, cx
	mov	di, offset aLine		; our destination buffer

lineLoop:

	; set up a run of pixels of a color to do

	mov	al, es:[si]		; get the control character from the control buffer
	inc	si

	mov	cl, al

	; decode RLE data
	.if	!(cl & REPEATC)
		; do run of unique bytes

		sub	runX, cx

		.repeat

			mov	al, es:[bx]		; get the color data
			inc	bx				
			mov	[di], al			
			inc	di
		.untilcxz

	.else				; it is either repeated color or repeated skip

		.if	cl & REPSKIP

			; it is repeated skip

  			and	cl, 03fh
			mov	al, skip

			sub	runX, cx	

			.repeat
				mov	[di], al	
				inc	di
			.untilcxz

			jmp	runDone

		.endif		; end of repeated skip

		; here we have a repeat color situation

		mov	al, es:[bx]		; get the color (only one for the sequence)
		inc	bx

		and	cl, 03fh

		sub	runX, cx

		.repeat

			mov	[di], al
			inc	di

		.untilcxz
	.endif
	
runDone:

	or		runX, 0			; is the line done?

	.if sign?
		invoke PanicMsgOutput, E_BUILD_LINE
		mov	decodeError, 1
	.endif

	jg		lineLoop

	ret

BuildLine endp

;------------------------------------------------------------------------
; BuildMirroredLine
;
; Build a line of the cel to the buffer "aLine" from the rle data in REVERSE
; order for mirroring
;
; This accomodates only non-scaled and mirrored cel.  It is exactly the same
; as BuildLine except that it starts at the end of the line and fills in
; to the beginning of the line.  All in all it seems the easiest/fastest
; way of mirroring
;

BuildMirroredLine proc near uses di cx
LOCAL runX:word

; es:0 = cel 
; ds = ss
; bx points to color buffer
; si points to control buffer
; dh high nibble has priority, low nibble f

assume	si:nothing, di:nothing

	; init loops vars

	mov	ax, lineWidth		; always the cel width in this case

	mov	runX, ax
	xor	cx, cx
	mov	di, offset aLine
	add	di, ax				; mov the pointer to the end of the cels's data output
	dec	di

mirroredLineLoop:

	; set up a run of pixels of a color to do

	mov	al, es:[si]		; get the control character from the control buffer
	inc	si

	mov	cl, al

	; decode RLE data
	.if	!(cl & REPEATC)
		; do run of unique bytes

		sub	runX, cx
		.repeat

			mov	al, es:[bx]
			inc	bx
			mov	[di], al
			dec	di
		.untilcxz

	.else				; it is either repeated color or repeated skip

		.if	cl & REPSKIP

			; it is repeated skip

  			and	cl, 03fh
			mov	al, skip

			sub	runX, cx

			.repeat
				mov	[di], al
				dec	di
			.untilcxz

			jmp	mirroredRunDone

		.endif		; end of repeated skip

		; here we have a repeat color situation

		mov	al, es:[bx]
		inc	bx

		and	cl, 03fh
		
		sub	runX, cx

		.repeat

			mov	[di], al
			dec	di

		.untilcxz
	.endif
	
mirroredRunDone:

	or		runX, 0			; is the line done?

	.if	sign?
		invoke PanicMsgOutput, E_MIRROR_BUILD
		mov	decodeError,3
	.endif

	jne	mirroredLineLoop

	ret

BuildMirroredLine endp


;------------------------------------------------------------------------
; StandardLine draws the decoded rle buffer from aLine to the virtual screen
; for standard non-scaled views (both normal and mirrored)

StandardLine proc near uses si es ds


	mov	si, offset aLine		; we are reading from the build line
	add	si, leftOffset			; left end clipping done

	mov	es, vSeg
	mov	ds, pcSeg

	; set DI to base address of line
	mov	di, ss:penY					; the screen line
	shl	di, 1
	mov	di, ss:baseTable[di]	; get an offset for the line from a stored table
	add	di, ss:penX				; plus offset to first pixel to be drawn

	mov	cx, ss:endX				; the count of pixels to be drawn
	mov	ah, ss:priMask

	.repeat							; for each pixel in the line

		mov	al, ss:[si]			; the actual color pixel
		inc	si

		; check for priority and skip color to determine whether to draw it
		.if	(al != ss:skip) && (dh >= [di])
			and	byte ptr [di], ah
			or		byte ptr [di], dl		; put the priority in the priority map

			; plot the color (remapping if required)
			mov	es:[di], al		; put the pixel to the visual map
		.endif
		
		inc	di

	.untilcxz


standardDone:

	ret

StandardLine endp


;------------------------------------------------------------------------
; EatLine
;
; Since their is no direct way to tell when a cel line ends directly from
; the rle data, it is necessary to decode a line of data to bypass it and
; advance the appropriate pointers.  This routine does that for all cel
; types

EatLine proc near uses di
LOCAL runX:word

; es:0 = cel 
; ds = ss
; bx points to color buffer at current offset
; si points to control buffer at current offset


assume	si:nothing, di:nothing

	; init loops vars

	mov	ax, lineWidth

	mov	runX, ax
	xor	ax, ax
	
eatLoop:
	
	mov	al, es:[si]		; get the control character from the control buffer
	inc	si

	; decode RLE data
	.if	!(al & REPEATC)
		; do run of unique bytes
		add	bx, ax	; use up al color bytes
	.else				; it is either repeated color or repeated skip
		.if al & REPSKIP
  			and	al, 03fh
			jmp	eatRunDone
		.endif		; end of repeated skip

		; here we have a repeat color situation
		inc	bx				; use one color byte
		and	al, 03fh
	.endif
	
eatRunDone:

	sub	runX, ax

	.if sign? 
		invoke PanicMsgOutput, E_EAT_LINE
		mov	decodeError, 2
	.endif
	
	jne	eatLoop

	ret

EatLine endp


;------------------------------------------------------------------------
; DrawSetup
;
; This routine is a utility functon for both DrawCel and ScaleDrawCel
; to setup the output rectangles and priority
;

DrawSetup proc near
LOCAL absPri:word		; priority value from caller, used for special stuff

; on entry
; ES:SI points to cel
; AX is X coord
; BX is Y coord
; CX must be ZERO
; DX is priority


assume	si:ptr Cel

	mov	ss:absPri, dx
				  
	; address the current port
	mov	di, rThePort
assume	di:ptr RGrafPort

	; offset top/left of cel
	add	ax, [di].origin.h
	mov	ss:penX, ax
;	mov	ss:hOff, ax
	add	bx, [di].origin.v
	mov	ss:penY, bx
	
	; make an offset copy of the ports portRect in theRect
	mov	bx, [di].origin.v
	mov	ax, [di].portRect.top
	add	ax, bx
	mov	theRect.top, ax
	mov	ax, [di].portRect.bottom
	add	ax, bx
	mov	theRect.bottom, ax
	mov	bx, [di].origin.h
	mov	ax, [di].portRect.left
	add	ax, bx
	mov	theRect.left, ax
	mov	ax, [di].portRect.right
	add	ax, bx
	mov	theRect.right, ax

	; build priority compare and store values
	mov	dx, absPri
	.if	dx >= MAXPRI
		mov	dx, 0ffffh
	.else
		shl	dl, 1
		shl	dl, 1
		shl	dl, 1
		shl	dl, 1
		mov	dh, dl
		or		dh, 0fh								; F in low nibble ensures it will be higher than any control
	.endif

	.if	absPri == -1
		mov	dl, 0
		mov	ss:priMask, 0ffh
	.else
		mov	ss:priMask, 0fh
	.endif


	; set skip high and low
	mov	al, es:[si].skipColor
	mov	skip, al

	; set horizontal run count
	mov	ax, es:[si].xDim
	mov	endX, ax

	.if scaled
		mov	ax, ss:realWidth
		mov	ss:lineWidth, ax
	.else
		mov	ss:lineWidth, ax
	.endif

	.if lineWidth > 320 
		invoke PanicMsgOutput, E_LINE_LENGTH
	.endif

	; do penY
	mov	ax, es:[si].yDim
	mov	endY, ax


	; point celPtr (double word) to data)
	mov	bx, word ptr es:[si].colorOffset	
	add	bx, objectOffset							; adjust to point to cel absolute offset
	mov	si, word ptr es:[si].dataOffset		; already points to cel absolute offset
	add	si, objectOffset

	mov	cx, penX
	mov	ss:leftOffset,0 

	; clip left side once for the entire cel
	.if cx < sword ptr theRect.left
		mov ax, theRect.left
		sub ax, cx
;		neg ax
		add	ss:endX, cx	; cx is negative
		mov	ss:leftOffset, ax		; a positive value 
		mov	ax, theRect.left
		mov	penX, ax		; ususualy 0

	.endif

	; clip right side once for the entire cel

	mov	ax, ss:penX
	add	ax, ss:endX
	
	.if ax > theRect.right
		sub	ax, theRect.right
		sub	ss:endX, ax
	.endif

	ret 

DrawSetup endp


;------------------------------------------------------------------------
;
; DrawCelZ is the main cel drawing loop for non-scaled views.
; It is called from DrawCel or DrawPicCel
;
; DS:  default data segment
; ES:  cel segment
; SI:  cel offset
; AX:  x coord
; BX:  y coord
; DX:  priority

DrawCelZ proc 


	call	DrawSetup		; setup the cel rectangle, priorities, etc.

	.if theViewNum == -1	; if it's a picture
		mov	skip, 0ffh
	.endif

	mov	cx, endX			; check to see if we are drawing the cel at all
	or		cx, cx
	.if sign? || zero?
		ret
	.endif

	.if theRect.top < 0 || theRect.bottom > 200
		invoke Panic, E_BAD_RECT
	.endif

	mov	ax,ss:penY

assume	si:nothing, di:nothing

	; we have separate loops for mirrored and non-mirrored data in order
	; to avoid a check for each line of the cel.  A little extra code, but
	; just a little faster

 	.if mirrored


		.while endY && ax < ss:theRect.bottom

			.if ax >= theRect.top	; ax = penY - check if we're off the top
				call BuildMirroredLine		; build a line of output data to aLine
				call StandardLine

			.else
				call EatLine	; won't be output so just advance indexes & counters
			.endif

			inc	ss:penY
			dec	ss: endY	; linesToDo 
			mov	ax,ss:penY
	
		.endw

	.else				; Normal, non-mirrored, non-scaled cel

		.while endY && ax < ss:theRect.bottom

			.if ax >= theRect.top	; ax = penY
				call BuildLine		; build a line of output data to aLine
		 		call StandardLine

			.else
				call EatLine	; won't be output so just advance indexes & counters
			.endif

			inc	ss:penY
			dec	ss: endY	; linesToDo 
			mov	ax, ss:penY

		.endw
	
	.endif

	ret


DrawCelZ endp


;------------------------------------------------------------------------
; ScaledLine
;
; This is the routine corresponding to StandardLine, but for scaled views.
; It outputs scaled views to the virtual screen from the decoded "aLine" buffer
;

ScaledLine proc near uses si di bx cx es ds


	mov	si, offset aLine		; we are reading from the built line

	mov	es, vSeg
	mov	ds, pcSeg

	; set DI to base address of line

	mov	di, ss:penY					; screen line
	shl	di, 1
	mov	di, ss:baseTable[di]		; get an offset for the line from a stored table
	add	di, ss:penX					; plus offset to first pixel to be drawn

	mov	bx, offset vm_deltaxs	; a table of pixels to read from aLine
	add	bx, ss:leftOffset			; the table is word length
	add	bx, ss:leftOffset
	
	mov	cx, ss:endX
	mov	ah, ss:priMask

	.while cx

		mov	si, ss:[bx]					; get the pixel # from the deltaxs buffer
		add	si, offset aLine		
		mov	al, ss:[si]		; the actual color pixel
		inc	bx							; the deltax buffer is words
		inc	bx

		; check for priority and skip color to determine whether to draw it

		.if  (al != ss:skip) && (dh >= [di])
			and	byte ptr [di], ah
			or		byte ptr [di], dl

				; plot the color (remapping if required)
			mov	es:[di], al
		.endif
		
		inc	di
		dec	cx

	.endw

	
	ret
ScaledLine endp


;------------------------------------------------------------------------
; ScaleDraw
;
; ScaleDraw is the main cel drawing loop for scaled views.  It corresponds
; to DrawCelZ for non-scaled views

ScaleDraw proc near
LOCAL lineCounter:word

assume	si:nothing, di:nothing

	; es = points to cel data
	; si = control data offset
	; ax = x coord
	; bx = y coord
	; cx = undefined
	; di = undefined (will be used to point to vm_deltays
	; ds = ss == local data
	; dx = adjusted priority 
	; endY and endX setup

	
	call	DrawSetup		; setup the cel rectangle, priorities, etc.

	mov	cx, endX			; check to see if we are drawing the cel at all
	or		cx, cx
	.if sign? || zero?
		ret
	.endif
		
	mov	di, offset vm_deltays	; table of lines to draw (duplicates or gaps probable!)
	xor	cx, cx
	mov	lineCounter, cx			; initialize

	mov	ax, ss:penY

	.while ax < theRect.top

		mov	cx, [di]					; get the scaled line number

		.if cx == lineCounter		; 
			
			call EatLine
			
			.while cx == ss:[di] 

				inc di
				inc di
				dec endY			; our count of lines to draw, not the cel height
			
				.if !endY 		
					jmp	scaleDone
				.endif

				inc	ss:penY

			.endw	; while we are drawing the same line

		.else
			call EatLine			; skip the line - view is shrunk
		.endif

		mov	ax,ss:penY			; 
		inc	lineCounter
	
	.endw

		

	.while endY > 0 && ax < ss:theRect.bottom

		mov	cx, [di]					; get the scaled line number

		.if cx == lineCounter		; 
			
			call BuildLine
			mov	ax, ss:penY
			
			.while cx == ss:[di] && ax < ss:theRect.bottom  ; we may have to draw it several times, but we decode once!

				call ScaledLine

				inc di
				inc di
				dec endY			; our count of lines to draw, not the cel height
			
				.if !endY 		
					jmp	scaleDone
				.endif

				inc	ss:penY
				mov	ax, penY

			.endw	; while we are drawing the same line

		.else
			call EatLine			; skip the line - view is shrunk
		.endif

		inc	lineCounter
		mov	ax,ss:penY			; 
	
	.endw

scaleDone:

	ret

ScaleDraw endp

;------------------------------------------------------------------------
; DrawPicCel
;
; this routine merely sets up some variables so we can safely call DrawCelZ

DrawPicCel proc uses si di  
	
	; ax = xOff
	; bx = yOff
	; cx = the picture's offset
	; DX = priority from picOverlayMask
	; es = picture
	; si = absolute cel visualHeader offset 
	; di = mirror flag

	; for debugging indicate this is a picture
	mov	theViewNum,-1
	mov	theLoopNum,-1
	mov	theCelNum, -1

	mov	ax, di	; mirrored flag
	mov	mirrored, al
	xor	ax, ax
	xor	bx, bx
  	
	mov	objectOffset, cx
	
	mov	scaled, al
	call	DrawCelZ
	ret


DrawPicCel endp

;------------------------------------------------------------------------
; DrawCel
;
; Entry point for drawing non-scaled views

DrawCel	proc	uses si di ,	view:word, loop:word, cel:word, pt:ptr RPoint, pri:word
;Draw requested cel of loop on color vMap in pri at X/Y
;offset pt to rThePort and clip drawing to the portRect

	push	ds
	pop	es
	
	xor	al, al
	mov	scaled, 0

	; get cel control in dx:ax, 
	; di=offset of the view header

	invoke	AssertViewPalette, view, 0

	invoke	GetCelPointer, view, loop, cel

	mov	objectOffset, di

	or		dx, dx
	.if	zero?
		ret
	.endif

	mov	es, dx
	mov	si, ax

	mov	di, pt
assume	di:ptr RPoint
	mov	ax, [di].h
	mov	bx, [di].v
assume	di:nothing
	mov	dx, pri
	call	DrawCelZ
	ret

DrawCel	endp

;------------------------------------------------------------------------
; ScaleDrawCel
;
; Entry point for drawing scaled views
;

ScaleDrawCel	proc	uses si di,	view:word, loop:word, cel:word, pt:ptr RPoint, pri:word, realXDim: word

	;Scaled Draw requested cel of loop on color vMap in pri at X/Y
	;offset pt to rThePort and clip drawing to the portRect

;	.if noScale
;		ret
;	.endif

	push	ds
	pop	es

	mov	scaled, 1	; set the flag

	; we need to keep track of how wide the cel was without scaling
	mov	ax, realXDim
	mov	realWidth, ax

	; get cel control in dx:ax, mirror in cx, bx=offset color data,
	; di=offset of the view header

	invoke	AssertViewPalette, view, 0
	invoke	GetCelPointer, view, loop, cel

	mov	objectOffset, di
	mov	es, dx
	mov	si, ax

	mov	di, pt
assume	di:ptr RPoint
	mov	ax, [di].h
	mov	bx, [di].v
assume	di:nothing
	mov	dx, pri
	call	ScaleDraw
	ret

ScaleDrawCel	endp


;----------------------------------------------------------------------------

GetCelWide proc	uses es si di,	view:word, loop:word, cel:word
;Return width of this cel

	invoke	GetCelPointer, view, loop, cel
	mov	si, ax
	mov	es, dx
	mov	ax, (Cel ptr es:[si]).xDim
	ret

GetCelWide endp

;----------------------------------------------------------------------------


GetCelHigh proc	uses si di, view:word, loop:word, cel:word 
;Return height of this cel

	invoke	GetCelPointer, view, loop, cel
	mov	si, ax
	mov	es, dx
	mov	ax, (Cel ptr es:[si]).yDim
	ret

GetCelHigh endp

;----------------------------------------------------------------------------

GetCelXOff proc	uses es si di,	view:word, loop:word, cel:word
;Return x offset of this cel

	invoke	GetCelPointer, view, loop, cel
	mov	si, ax
	mov	es, dx
	mov	ax, (Cel ptr es:[si]).xOff
	ret

GetCelXOff endp

;----------------------------------------------------------------------------

GetCelYOff proc	uses es si di,	view:word, loop:word, cel:word
;Return y offset of this cel

	invoke	GetCelPointer, view, loop, cel
	mov	si, ax
	mov	es, dx
	mov	ax, (Cel ptr es:[si]).yOff
	ret

GetCelYOff endp

;----------------------------------------------------------------------------

GetCelRect proc uses es si di,\
	view:word, loop:word, cel:word, x:word, y:word, z:word, rPtr:ptr RRect
;Produce a rectangle in passed structure for desired cel

	invoke	GetCelPointer, view, loop, cel
	mov	si, ax
	mov	es, dx

	mov	di, rPtr

assume	si:ptr Cel
assume	di:ptr RRect

	; offset horizontal position of passed point
	; prepare value in CX
	mov	cx, x
	mov	ax, es:[si].xDim
	shr	ax, 1
	sub	cx, ax

	; xOff is subtracted from this to move it right or left of point
	mov	ax, es:[si].xOff

	; if MIRRORED we negate this value to offset in proper direction

	.if	mirrored
		neg	ax
	.endif
	add	cx, ax
	mov	[di].left, cx

	; use this to set right of rectangle
	add	cx, es:[si].xDim
	mov	[di].right, cx

	; now do vertical offset (moves it down) unsigned
	mov	ax, es:[si].yOff
	or		ax, ax			; set flags
	.if sign?
;		neg	ax
		add	ax, 255
	.endif

	;the rectangle that encompasses this point has a bottom one line lower
	;Z represents height above ground so we subtract from Y to move it up
	mov	cx, y
	sub	cx, z
	inc	cx
	add	cx, ax
	mov	[di].bottom, cx

	; now fix top for height of cel
	sub	cx, es:[si].yDim
	mov	[di].top, cx
	ret

assume	si:nothing
assume	di:nothing

GetCelRect endp

;----------------------------------------------------------------------------


GetNumLoops proc uses es si, view:word
;Return number of loops in this view

	mov	si, view
	les	si, [si]
	xor	ah, ah
	mov	al, (View ptr es:[si]).numLoops
	ret

GetNumLoops endp


;----------------------------------------------------------------------------
; GetNumCels
; 
; Find out how many cels the loop has. It's somewhat harder now with the new
; view format
;

GetNumCels proc uses es si, view:word, loop:word
;Return number of cels in this loop of view

	; validate and point to loop base
	mov	bx, loop

altLoopLoop:

	mov	si, view
	les	si, [si]

	xor	ah, ah
	mov	al, (View ptr es:[si]).numLoops
	.if	bx >= ax
		mov	bx, ax
		dec	bx
	.endif

assume si:ptr View

	xor	ah, ah
	mov	al, es:[si].loopHeaderSize
	mul	bl		; find the loop header offset
	add	ax, es:[si].viewHeaderSize
	add	ax, 2			; because the viewheadersize word not included in above
	add	si, ax		; the loop header offset
	
assume si:ptr Loop
	
	mov	bl, es:[si].altLoop
	.if	bl != -1
		mov	bl, es:[si].altLoop
		jmp	altLoopLoop		; do it over with adjusted loop number
	.endif
 
	mov	al, es:[si].numCels
	xor	ah, ah

assume si:nothing

	ret

GetNumCels endp

;---------------------------------------------------------------------------
; SetNewDim
;
; Adjust the cel x and y dim and x-y offsets for scaled views
; 
SetNewDim	proc uses si di, view:word, lop:word, cel:word ,newxdim:word, newydim:word, newxoff:word, newyoff:word

	push 	ds
	pop	es


	; get cel pointer
	invoke	GetCelPointer, view, lop, cel	 	
		
	mov	si, ax
	mov	es, dx

	; Set new dimensions

	mov	ax, newxdim
	mov	dx, newydim
   mov   bx, newxoff
   mov   cx, newyoff

	assume	si:ptr Cel
	mov	es:[si].xDim, ax
	mov	es:[si].yDim, dx
   mov   es:[si].xOff, bx
   mov   es:[si].yOff, cx
	assume	si:nothing

	ret
	
SetNewDim endp

;---------------------------------------------------------------------------

DrawMagCursor proc uses es di  si magData:ptr MagCursorStruct,pCursor:ptr pseudoCursor, xpos:word, ypos:word
LOCAL cursorXPtr:word, cursorYPtr:word, bigPicXDim:word, \
		videoXPtr:word, videoYPtr:word, hToDo:word, vToDo:word, \	
		magPtrX:word, magPtrY:word, curXDim:word, curSkip:byte, \
		magSkip:byte, magSegment:word, magOffset:word

; Setup routine to initialize variables and obtain data pointers
; Init variables
	mov	di, pCursor
	mov	si, magData
assume	di:ptr pseudoCursor		
assume	si:ptr MagCursorStruct	
	mov	ax, [di].xDim
	mov	curXDim, ax
	mov	al, [di].skipColor
	mov	curSkip, al
	mov	al, [si].replaceColor
	mov	magSkip, al

	;calc cursorXPtr and videoXPtr
	mov	ax, [si].curLeft				;cursorXPtr init
	mov	bx, xpos
	sub	bx,  [di].xHot		;videoXPtr init
	.if sword ptr	bx < [si].curLeft 
		sub	ax, bx
		mov	bx, [si].curLeft
		mov	cursorXPtr, ax
	.else
		mov	cursorXPtr, 0
	.endif
	mov	videoXPtr, bx

	;calc pointer into magnified picture data	 for x coord
	mov	ax, xpos
	sub	ax, [si].curLeft
	mul	[si].magLevel
	sub	ax,  [di].xHot
	.if	sword ptr ax < 0
		mov	magPtrX, 0
	.else
		mov	magPtrX, ax
	.endif

	;calc cursorYPtr and videoYPtr
	mov	ax, [si].curTop				;cursorYPtr init
	mov	bx, ypos
	sub	bx,  [di].yHot		;videoYPtr init
	.if  sword ptr	bx < [si].curTop
		sub	ax, bx
		mov	bx, [si].curTop
		mov	cursorYPtr, ax
	.else
		mov	cursorYPtr, 0
	.endif
	mov	videoYPtr, bx

	;calc pointer into magnified picture data	 for y coord

	mov	ax, ypos
	sub	ax, [si].curTop
	mul	[si].magLevel
	sub	ax,  [di].yHot
	.if	sword ptr ax < 0
		mov	magPtrY, 0
	.else
		mov	magPtrY, ax
	.endif

	;calc hToDo
	mov	ax,  [di].xHot
	sub	ax, cursorXPtr			;to do on left of Hot Spot
	mov	bx, [si].curRight
	sub	bx, xpos
 	mov	cx, [di].xDim
	sub	cx,  [di].xHot   ;to do on right of Hot Spot
	cmp	bx, cx
	jle	stdgotright
	mov	bx, cx				   ;to do on right of Hot Spot
stdgotright:
	add	ax, bx
	mov	hToDo, ax

	;calc vToDo
	mov	ax,  [di].yHot
	sub	ax, cursorYPtr			;to do on top of Hot Spot
	mov	bx, [si].curBottom
	sub	bx, ypos
 	mov	cx, [di].yDim
	sub	cx,  [di].yHot   ;to do on bottom of Hot Spot
	cmp	bx, cx
	jle	stdgotbottom
	mov	bx, cx				   ;to do on bottom of Hot Spot
stdgotbottom:
	add	ax, bx
	mov	vToDo, ax
	
	mov	bx, magData
	assume	bx: ptr MagCursorStruct		

	push	ds
	mov	si, [bx].cursorDataOff
	mov	ax, [bx].pictDataSeg
	mov	ss:magSegment, ax
	mov	ax, [bx].pictDataOff
	mov	ss:magOffset, ax
	mov	ax, [bx].picXDim
	mov	ss:bigPicXDim, ax
	mov	ax, [bx].cursorDataSeg 
	mov	ds,ax

	; load es with video virtual map

	mov	ax, ss:vSeg
	mov	es, ax

	mov	di, ss:videoYPtr
	shl	di, 1
	mov	di, ss:baseTable[di]
	add	di, ss:videoXPtr
	
	; get start byte for cursor

	mov	ax, ss:cursorYPtr
	mul	ss:curXDim
	add	si, ax
	add	si, ss:cursorXPtr
	

	; get start byte for magnified picture buffer

	mov	ax, ss:magPtrY
	mul	ss:bigPicXDim
	add	ax, ss:magPtrX
	add	ax, ss:magOffset
	mov	bx, ax

	; Main drawing loop
	assume bx:nothing
	.while	ss:vToDo > 0
		mov	cx, ss:hToDo
		push	si
		push	di
		push	bx
		; load dx with skip values
		mov	dh, ss:curSkip
		mov	dl, ss:magSkip
		.while	cx > 0
			lodsb
	  		.if al !=  dh ;ss:curSkip		; Check for regular skip
				.if al != dl ;ss:magSkip
	  				stosb	
				.else			  ; draw data from magnified data buffer
					push	ds
					mov	ds, ss:magSegment
					mov	al, [bx]
					pop	ds
					stosb
				.endif
			.else
 				inc	di
 			.endif
			inc	bx
			dec	cx
		.endw
		pop	bx
		pop	di
		pop	si
		;increment pointers
		add	si, ss:curXDim
		add	di, VROWBYTES
		add	bx, ss:bigPicXDim
		dec	ss:vToDo
	.endw
	pop 	ds
	ret
DrawMagCursor endp

;--------------------------------------------------------------------------

RIsItSkip proc	uses si di, view:word, loop:word, cel:word, vOffset:word, hOffset:word

	invoke	GetCelPointer, view, loop, cel ;sets ax,dx with address, di view offset
	mov	si, ax
	mov	es, dx		; cel seg
	
assume	si:ptr Cel
	mov	ax, es:[si].yDim
	mov	endY, ax

	mov	cl, es:[si].skipColor	; using cl becauses it's saved by buildLine

	; setup stuff needed by buildLine
	mov	ax, es:[si].xDim
	mov	lineWidth, ax
	mov	bx, di					; the view offeset
	add	bx, word ptr es:[si].colorOffset
	add	di, word ptr es:[si].dataOffset
	mov	si, di
  mov   skip, cl

assume si:nothing

	xor	di, di		; use as a line counter becauses its saved by buildLine

	.while endY && di <= vOffset	; build lines until we reach the proper one
		call	BuildLine
		inc	di
		dec	endY
	.endw

	; now, we should have the line we want in aLine

	mov	si, offset aLine

	; point to the relevant pixel

	.if mirrored
		add	si, lineWidth
		sub	si, hOffset
	.else
		add	si, hOffset
	.endif

	.if cl == byte ptr ss:[si]
		mov	ax, 1
	.else
		xor	ax,ax
	.endif

	ret	

RIsItSkip	endp

;----------------------------------------------------------------------------

AssertViewPalette proc uses ax dx si di, view:word, flag:word

local PalPtr:dword

	push	ds
	pop	es

	invoke	GetCelPointer, view, 0, 0

	or	dx, dx
	.if zero?
		ret
	.endif

	mov	es, dx  		; the view's segment

assume di:ptr View

	.if word ptr es:[di].paletteOffset == 0  	; done if the view has no palette
		ret
	.endif

	; di contains the view's offset
	add	di, word ptr es:[di].paletteOffset	


	; if the flag is set, FORCE the palette update, otherwise depend on the
	; palStamp to determine if we should do the palette update

assume di:ptr danPalette		



; ret	 ; for LB2 Only - Can't Handle Assert Palette


	.if !flag
		mov	ax, palStamp
		.if word ptr es:[di].validStamp	== ax
			ret
		.endif
	.endif

	mov	word ptr es:[di].validStamp, ax		; update the palette stamp

assume di:ptr View

	mov	word ptr PalPtr, di
	mov	word ptr PalPtr + 2,dx
	lea	si, PalPtr

; the following call is safe only as long as RSetDanPalette does not 
; allocate memory or load resources! (which it currently (6/92) doesn't)

	invoke	RSetDanPalette, si

	.if palVaryOn
		invoke PaletteUpdate, NOSETCLUT, 0
	.endif
	
	.if !picNotValid
  		.if NumberColors == 16
			invoke SetCLUT, addr sysPalette, ds, FALSE
	   .endif
	.endif

	ret

AssertViewPalette endp


;---------------------------------------------------------------------------


DisplayBlock proc uses si di bp ds, theBuffer:far ptr, xOffset:word, yOffset:word, xDim:word, yDim:word

RectDisplay equ 26


	mov	di, word ptr theBuffer + 2
	mov	si, word ptr theBuffer
	mov	ax, xOffset
	mov	bx, yOffset
	mov	cx, xDim
	mov	dx, yDim

	mov	bp, RectDisplay
	call	dword ptr Display

	ret

DisplayBlock endp

;---------------------------------------------------------------------------
	
FillVideoPage proc uses si di bp ds, frame:far ptr

RectDisplay equ 26

	
	mov	ax, word ptr frame
	mov	dx, word ptr frame + 2

	mov	bp, 	RectDisplay
	call	dword ptr Display

	ret

FillVideoPage endp
;---------------------------------------------------------------------------

SetMode equ 28

SetVideoMode proc uses si di bp, mode:word

	mov	dx, mode
	mov	bp, SetMode
	call	dword ptr Display
	ret

SetVideoMode endp
;---------------------------------------------------------------------------

DisplayPageAddress equ 30

ShowPage proc uses si di bp, pageOffset:word

	mov	bx, pageOffset
	mov	bp, DisplayPageAddress
	call	dword ptr Display

	ret

ShowPage endp

;---------------------------------------------------------------------------


CopyPage proc uses si di bp, destPage:word, srcPage:word

	mov	di, destPage
	mov	si, srcPage
	mov	bp, 32			; copy page routine
	call	dword ptr Display
	ret

CopyPage endp
;---------------------------------------------------------------------------


;ReadPal proc uses ax bx cx dx es

;		mov	ax, ss
;		mov	es, ax
;		mov	ax, 1017h	; read block of color regs
;		mov	bx, 0
;		mov	cx, 100
;		mov	dx, offset palBuff
;		int	10h

;		ret

;ReadPal endp

;---------------------------------------------------------------------------

SetupVMap proc uses di bp, sourcePage:word

	mov	bx, sourcePage
	mov	dx, vSeg
	mov	bp, 34				; Copy2Vmap 
	call	dword ptr Display

	; clear the priority/control map
	mov	ax, pcSeg
	mov	es, ax
	xor	ax, ax
	xor	di, di
	mov	cx, 32000
	rep	stosw

	ret

SetupVMap endp


	END

