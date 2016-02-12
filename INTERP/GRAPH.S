;-------------------------------------------------------------------
; GRAPH.S 
; 
; Routines herein provide manager level
; manipulation of the virtual bitmaps.
; low level graphics functions are located in GRFUNC.ASM
; Also provide C binding to device driver routine.
; 
; Started on 5-8-87
; by Robert E. Heitman
;
; Modifications:  
;   04/03/90 Abdul.  Changed to use .MODEL directive and simplified
;                    segments.  Microsoft MASM 5.1.
;   09/03/90 Mickie Lee
;                  Adding Japanese Display routine
;-------------------------------------------------------------------


include	macro.i
include	graph.i
	
include	fileio.i
include	resource.i
include	restypes.i
include	memmgr.i
include	mouse.i
include	intrpt.i
include	video.i
include	text.i


ClearChar		proto	near
OffsetPoint		proto near
SInsetRect		proto near
InRect			proto near




.DATA

Display		dword			?
rThePort		GPPtr			?
defPort		RGrafPort	<>
theRect		RRect			<>						; local normalized rectangle
oldMode		word			?

wordBits		word	0001h, 0002h, 0004h, 0008h, 0010h, 0020h, 0040h, 0080h
				word	0100h, 0200h, 0400h, 0800h, 1000h, 2000h, 4000h, 8000h


.CODE


;-------------------------------------------------------------------
; try to set the font of this port (do a ResLoad and check for error)
;-------------------------------------------------------------------

RSetFont proc uses es, font:word

	invoke	ResLoad, RES_FONT, font
	.if	ax
		; dereference the handle
		mov	bx, ax
		les	bx, [bx]
		mov	ax, (Font ptr es:[bx]).pointSize

assume	bx:ptr RGrafPort
		mov	bx, rThePort
		mov	[bx].fontSize, ax
		mov	ax, font
		mov	[bx].fontNum, ax
assume	bx:nothing
	.endif

	ret
	
RSetFont endp


;-------------------------------------------------------------------
; passed a pointer to a RGrafPort, init all fields to defaults
;-------------------------------------------------------------------

ROpenPort proc uses si di, gPort:GPPtr

assume	di:ptr RGrafPort

	; set the font to system font
	push	rThePort
	mov	di, gPort
	mov	rThePort, di
	mov	[di].fontNum, 0
	mov	[di].fontSize, 8
	invoke	RSetFont, 0
	pop	rThePort

	mov	di, gPort
	mov	[di].origin.v, 0
	mov	[di].origin.h, 0
	mov	[di].pnLoc.h, 0
	mov	[di].pnLoc.v, 0
	mov	[di].txFace, 0
	mov	[di].fgColor, vBLACK
	mov	[di].bkColor, vWHITE
	mov	[di].pnMode, SRCOR
	push	ds
	pop	es
	lea	di, [di].portRect
	lea	si, bounds
	mov	cx, 4
	rep	movsw

	ret

assume	di:nothing

ROpenPort endp


;-------------------------------------------------------------------
; load video driver and do general initialization
; also set rThePort to fresh defPort
;-------------------------------------------------------------------

CInitGraph proc	driver:word

	; Load and lock the video driver
	invoke	LoadHandle, driver
	.if	ax
		; Lock and dereference the handle we got
		mov	bx, ax
		les	ax, [bx]
		ses	Display, ax
		invoke	LockHandle, bx

		; initialize the display driver
		call	InitDisplay

		.if ax == -1
			xor	ax, ax		; error return (added for vesa driver)
			ret
		.endif

		mov	oldMode, ax
	
		; call resolution specific initialization
		call	InitGraph

		; create default port
		lea	ax, defPort
		mov	rThePort, ax
		invoke	ROpenPort, ax

		; return 1 for true
		mov	ax, 1
	.endif

	; all done. AX is zero if we failed in any way
	ret

CInitGraph endp


;-------------------------------------------------------------------
; point passed pointer to rThePort
;-------------------------------------------------------------------

RGetPort proc	pPtr:word

	mov	bx, pPtr
	mov	ax, rThePort
	mov	[bx], ax
	ret

RGetPort endp


;-------------------------------------------------------------------
; point rThePort to passed address
;-------------------------------------------------------------------
RSetPort proc	pPtr:word

	mov	ax, pPtr
	mov	rThePort, ax
	ret

RSetPort endp


;-------------------------------------------------------------------
; set the origin of rThePort to X/Y
; x must be even
;-------------------------------------------------------------------

RSetOrigin proc x:word, y:word

assume	bx:ptr RGrafPort

	mov	ax, x
	and	ax, not 1
	mov	bx, rThePort
	mov	[bx].origin.h, ax
	mov	ax, y
	mov	[bx].origin.v, ax
	ret

assume	bx:nothing

RSetOrigin endp


;-------------------------------------------------------------------
; move pen loc to x/y
;-------------------------------------------------------------------
RMoveTo proc	x:word, y:word

assume	bx:ptr RGrafPort

	mov	bx, rThePort
	mov	ax, x
	mov	[bx].pnLoc.h, ax
	mov	ax, y
	mov	[bx].pnLoc.v, ax
	ret

assume	bx:nothing

RMoveTo endp


;-------------------------------------------------------------------
; move penloc by delta x/y
;-------------------------------------------------------------------

RMove proc	x:word, y:word

assume	bx:GPPtr

	mov	bx, rThePort
	mov	ax, x
	add	[bx].pnLoc.h, ax
	mov	ax, y
	add	[bx].pnLoc.v, ax
	ret

assume	bx:nothing

RMove endp


;-------------------------------------------------------------------
; set pen's foreground color
;-------------------------------------------------------------------

PenColor proc	mode:word

	mov	bx, rThePort
	mov	ax, mode
	mov	(RGrafPort ptr [bx]).fgColor, ax
	ret

PenColor endp

	
;-------------------------------------------------------------------
; set pen's background color
;-------------------------------------------------------------------

RBackColor proc	mode:word

	mov	bx, rThePort
	mov	ax, mode
	mov	(RGrafPort ptr [bx]).bkColor, ax
	ret

RBackColor endp


;-------------------------------------------------------------------
; set pen's transfer mode
;-------------------------------------------------------------------

RPenMode proc	mode:word

	mov	bx, rThePort
	mov	ax, mode
	mov	(RGrafPort ptr [bx]).pnMode, ax
	ret

RPenMode endp


;-------------------------------------------------------------------
; set txFace
;-------------------------------------------------------------------

RTextFace proc	face:word

	mov	bx, rThePort
	mov	ax, face
	mov	(RGrafPort ptr [bx]).txFace, ax
	ret

RTextFace endp


;-------------------------------------------------------------------
; return point size of port's current font or over ride
;-------------------------------------------------------------------
GetPointSize proc
	
	mov	bx, rThePort
	mov	ax, (RGrafPort ptr [bx]).fontSize
	ret

GetPointSize endp


;-------------------------------------------------------------------
; over ride the point setting of the current font
;-------------------------------------------------------------------

SetPointSize proc	siz:word

	mov	bx, rThePort
	mov	ax, siz
	mov	(RGrafPort ptr [bx]).fontSize, ax
	ret

SetPointSize endp


;-------------------------------------------------------------------
; return the font number of the current port
;-------------------------------------------------------------------

GetFont proc

	mov	bx, rThePort
	mov	ax, (RGrafPort ptr [bx]).fontNum
	ret

GetFont endp


;-------------------------------------------------------------------
; end the graphics
;-------------------------------------------------------------------

CEndGraph proc

	call	EndGraph
	invoke	EndDisplay, oldMode
	ret

CEndGraph endp


;-------------------------------------------------------------------
; call fill rect to xor white with current color
;-------------------------------------------------------------------

RInvertRect proc	uses si, r:ptr RRect
; call fill rect to xor white with current color

assume	si:GPPtr

	mov	si, rThePort
	push	[si].pnMode
	mov	[si].pnMode, srcInvert
	invoke	RFillRect, r, VMAP, [si].fgColor, [si].bkColor, 0
	pop	[si].pnMode
	ret

assume	si:nothing

RInvertRect endp


;-------------------------------------------------------------------
; call fill rect to fill in foreground color
;-------------------------------------------------------------------

RPaintRect proc	r:ptr RRect

	mov	bx, rThePort
	invoke	RFillRect, r, VMAP, (RGrafPort ptr [bx]).fgColor, 0, 0
	ret

RPaintRect endp


;-------------------------------------------------------------------
; call fill rect to fill in background color
;-------------------------------------------------------------------

REraseRect proc	r:ptr RRect

	mov	bx, rThePort
	invoke	RFillRect, r, VMAP, (RGrafPort ptr [bx]).bkColor, 0, 0
	ret

REraseRect endp


;-------------------------------------------------------------------
; return total width of this text
;-------------------------------------------------------------------

RTextWidth proc uses si di es, s:ptr byte, first:word, cnt:word, defaultFont:word
	
	; get a handle to current font
	mov	di, rThePort
	invoke	ResLoad, RES_FONT, (RGrafPort ptr [di]).fontNum

	; dereference the handle
	mov	di, ax
	les	di, [di]
assume	di: ptr Font

	; set up to add widths
	mov	si, s
	add	si, first
	mov	dx, 0										; accumulated width
	mov	cx, cnt
	.if	cx
		.repeat
			lodsb
			.if (al == CTRL_CHAR)				;Control code character
				.if (cx > 1)						;Anything left to read?
					lodsb
					dec	cx
					cmp	al, CTRL_CHAR
					je		AddCharWidth			;No control code found -> want CTRL_CHAR
					.if ((al == 'f') && (cx > 1))	;Font change request?
					
						push	dx						;Save text width

						lodsb
						dec	cx
						.if (al == CTRL_CHAR)	;If no font given, use default

							mov	bx, defaultFont

						.else

							mov	bx, 0				;Get font table index
							.while ((al >= '0') && (al <= '9') && (cx > 1))
								xor	ah, ah
								sub	al, '0'
								push	ax
								mov	ax, 10
								mul	bx
								pop	dx
								add	ax, dx
								mov	bx, ax
								lodsb
								dec	cx
							.endw

							shl	bx, 1
							mov	bx, textFonts[bx]	;Get actual font requested

						.endif
						
						push	ax						;Save last character
						push	bx						;Save Font number
						
						; Set new font
						push	cx						;Save count
						invoke	ResLoad, RES_FONT, bx
						pop	cx						;Restore count
							
assume	di: nothing
						; dereference the handle
						mov	di, ax
						les	di, [di]
assume	di: ptr Font
						pop	ax						;Restore font number
assume	bx:ptr RGrafPort
						mov	bx, rThePort
						mov	[bx].fontNum, ax
						mov	ax, es:[di].pointSize
						mov	[bx].fontSize, ax
assume	bx:nothing
						
						pop	ax						;Restore last character
						pop	dx						;Restore text width
					.endif
					.while ((al != CTRL_CHAR) && (cx > 1))	;Go to end of control code
						lodsb
						dec	cx
					.endw
				.endif
			.else
AddCharWidth:
				mov	bl, al
				xor	bh, bh

				; index to character and get width
				shl	bx, 1
				mov	bx, es: [di][bx].charRecs	; offset to offset
				mov	al, byte ptr es: [di][bx] 	; offset to character record
				cbw
				add	dx, ax
			.endif
		dec	cx
		.until zero?
	.endif

	; return total width in AX
	mov	ax, dx
	ret

assume	di:nothing

RTextWidth endp



;-------------------------------------------------------------------
; return width in pixels of the character
;-------------------------------------------------------------------
RCharWidth proc uses si es, cCode:word

	and	word ptr cCode, 0ffh

	mov	si, rThePort
	invoke	ResLoad, RES_FONT, (RGrafPort ptr [si]).fontNum

	; dereference the handle
	mov	si, ax
	les	si, [si]
	mov	bx, cCode
	shl	bx, 1
	add	si, (Font ptr es:[si])[bx].charRecs
	mov	al, es:[si]
	xor	ah, ah
	ret

RCharWidth endp



;-------------------------------------------------------------------
; return height in pixels of the character
;-------------------------------------------------------------------

CharHeight proc uses si es, cCode:word

	and	word ptr cCode, 0ffh

	mov	si, rThePort
	invoke	ResLoad, RES_FONT, (RGrafPort ptr [si]).fontNum

	; dereference the handle
	mov	si, ax
	les	si, [si]
	mov	bx, cCode
	shl	bx, 1
	add	si, (Font ptr es:[si])[bx].charRecs
	mov	al, es:[si + 1]
	xor	ah, ah
	ret

CharHeight endp



;-------------------------------------------------------------------
; draw a graphics character into visual map at current pnLoc
; and advance pnLoc by characters width
;-------------------------------------------------------------------

RDrawChar proc cCode:word

	and	word ptr cCode, 0ffh
	mov	ax, cCode
	call	ClearChar
	call	StdChar

	; now move pen loc to right
	invoke	RCharWidth, cCode
	mov	bx, rThePort
	add	(RGrafPort ptr [bx]).pnLoc.h, ax
	ret

RDrawChar endp



;-------------------------------------------------------------------
; draw a graphics character into visual map at current pnLoc
; show the bits and advance pnLoc
;-------------------------------------------------------------------

ShowChar proc uses si, cCode:word

assume	si:ptr RGrafPort

	and	word ptr cCode, 0ffh
	mov	ax, cCode
	call	ClearChar
	call	StdChar

	; show what we did
	mov	si, rThePort
	mov	ax, [si].pnLoc.h
	mov	theRect.left, ax

	; update penLoc
	invoke	RCharWidth, cCode
	add	[si].pnLoc.h, ax	; move the pen
	mov	ax, [si].pnLoc.h	; use for right of showrect
	mov	theRect.right, ax

	; set vertical component of show rectangle
	mov	bx, [si].pnLoc.v
	mov	theRect.top, bx
	invoke	CharHeight, cCode
	add	bx, ax
	mov	theRect.bottom, bx
	invoke	ShowBits, addr theRect, VMAP
	
	ret

assume	si:nothing

ShowChar endp



;-------------------------------------------------------------------
; if pnMode = SRCCOPY we erase the rectangle
; it will occupy width by fontSize
;-------------------------------------------------------------------

ClearChar proc near

assume	bx:ptr RGrafPort
	mov	bx, rThePort
	.if	[bx].pnMode == SRCCOPY
		push	ax
		push	ax
		mov	ax, [bx].pnLoc.v
		mov	theRect.top, ax
		add	ax, [bx].fontSize
		mov	theRect.bottom, ax

		; use width of this character
		mov	ax, [bx].pnLoc.h
		mov	theRect.left, ax
		mov	theRect.right, ax
		pop	ax
		invoke	RCharWidth, ax
		add	theRect.right, ax
		invoke	REraseRect, addr theRect
		pop	ax
	.endif
	ret
assume	bx:nothing

ClearChar endp

;-------------------------------------------------------------------
; high level entry
;-------------------------------------------------------------------

ROffsetRect proc	r:ptr RRect, x:word, y:word

assume	bx:ptr RRect

	mov	bx, r
	mov	ax, x
	add	[bx].right, ax
	add	[bx].left, ax
	mov	ax, y
	add	[bx].top, ax
	add	[bx].bottom, ax
	ret

assume	bx:nothing

ROffsetRect endp


;-------------------------------------------------------------------
; add the origin of port at DI to rectangle at SI
;-------------------------------------------------------------------

SOffsetRect proc

assume	di:GPPtr
assume	si:ptr RRect

	mov	ax, [di].origin.v
	add	[si].top, ax
	add	[si].bottom, ax
	mov	ax, [di].origin.h
	add	[si].left, ax
	add	[si].right, ax
	ret

assume	di:nothing
assume	si:nothing

SOffsetRect endp


;-------------------------------------------------------------------
; add the origin of port at DI to point at SI
;-------------------------------------------------------------------

OffsetPoint proc near

assume	di:GPPtr
assume	si:ptr RPoint

	mov	ax, [di].origin.v
	add	[si].v, ax
	mov	ax, [di].origin.h
	add	[si].h, ax
	ret

assume	di:nothing
assume	si:nothing

OffsetPoint endp


;-------------------------------------------------------------------
; inset (or outset) pointed to rect
;-------------------------------------------------------------------

RInsetRect proc uses si, r:ptr RRect, x:word, y:word

	mov	si, r
	mov	ax, x
	mov	bx, y
	call	SInsetRect
	ret

RInsetRect endp


;-------------------------------------------------------------------

SInsetRect proc near

assume	si:ptr RRect

	add	[si].left,  ax
	sub	[si].right, ax
	add	[si].top,    bx
	sub	[si].bottom, bx
	ret

assume	si:nothing

SInsetRect endp


;-------------------------------------------------------------------
; move rectangle so origin is at x/y
;-------------------------------------------------------------------

MoveRect proc	uses si, r:ptr RRect, x:word, y:word

assume	si:ptr RRect

	mov	si, r
	mov	ax, x
	sub	ax, [si].left
	mov	bx, y
	sub	bx, [si].top
	add	[si].left, ax
	add	[si].right, ax
	add	[si].top, bx
	add	[si].bottom, bx
	ret

assume	si:nothing

MoveRect endp


;-------------------------------------------------------------------
; return true if point at SI is in RRect at di
;-------------------------------------------------------------------

InRect proc near

assume	si:ptr RPoint
assume	di:ptr RRect

	mov	ax, [si].v
	mov	bx, [si].h
	.if	ax >= [di].top && ax < [di].bottom && \
			bx >= [di].left && bx < [di].right
		clc
	.else
		stc
	.endif
	ret

assume	si:nothing
assume	di:nothing

InRect endp


;-------------------------------------------------------------------
; C interface to SectRect.  Return true if a valid rectangle remains
;-------------------------------------------------------------------

CSectRect proc uses si di, r1:ptr RRect, r2:ptr RRect, r3:ptr RRect

	mov	si, r1
	mov	di, r2
	mov	bx, r3
	call	RSectRect

	;condition AX for validity of rectangle
	mov	ax, 0
	.if	carry?
		inc	ax
	.endif
	
	ret
	
CSectRect endp


;-------------------------------------------------------------------
; return intersection of passed rectangles in source rectangle
; SI points at source, DI at clpping rectangle, result goes at BX
; Logic: If source is inside clip, ok; else move clip to src
;start clipping	 with top/left
;-------------------------------------------------------------------

RSectRect proc

assume	ax:sword
assume	si:ptr RRect
assume	di:ptr RRect
assume	bx:ptr RRect

	mov	ax, [si].top
	.if	ax < [di].top
		mov	ax, [di].top
	.elseif	ax > [di].bottom
		mov	ax, [di].bottom
	.endif
	mov	[bx].top, ax

	mov	ax, [si].left
	.if	ax < [di].left
		mov	ax, [di].left
	.elseif 	ax > [di].right
		mov	ax, [di].right
	.endif
	mov	[bx].left, ax

	mov	ax, [si].bottom
	.if	ax < [di].top
		mov	ax, [di].top
	.elseif	ax > [di].bottom
		mov	ax, [di].bottom
	.endif
	mov	[bx].bottom, ax

	mov	ax, [si].right
	.if	ax < [di].left
		mov	ax, [di].left
	.elseif	ax > [di].right
		mov	ax, [di].right
	.endif
	mov	[bx].right, ax

	;Test if the rectangle left is any good
	mov	ax, [bx].left
	cmp	ax, [bx].right
	.if	sign?
		mov	ax, [bx].top
		cmp	ax, [bx].bottom
	.endif

	ret

assume	ax:nothing
assume	bx:nothing
assume	si:nothing
assume	di:nothing

RSectRect endp


;-------------------------------------------------------------------
; get an input rectangle into local (rect at SI)
;-------------------------------------------------------------------

GetTheRect proc	uses si

	lea	di, theRect
	mov	cx, 4
	rep	movsw
	ret

GetTheRect endp



GetCursorData proc uses si di bp, data:word
; Copy cursor data into passed buffer
; Buffer size should be the maximum cursor size (910 bytes).

	push	es
	push	ds

	push	ds
	push	data

	; point to cursor area (return in dx:ax)
	mov	bp, V_SETCUR
	call	Display
	mov	ds, dx
	mov	si, ax

	pop	di
	pop	es

	lodsw					; xDim
	stosw
	mov	dx, ax
	lodsw					; yDim
	stosw
	mul	dx				; ax contains data size
	mov	cx, 3
	rep	movsw			; xOff, yOff, skip, unused
	mov	cx, ax
	rep	movsb			; copy data

	pop	ds
	pop	es

	;show cursor (V_SETCUR hides cursor)
	mov	bp, V_SHOWCUR
	call	Display

	ret

GetCursorData endp



SetCursorData proc uses si di bp, data:word
; Move data into cursor area, then redraw cursor

	push	es
	push	ds

	push	data

	; point to cursor area (return in dx:ax)
	mov	bp, V_SETCUR
	call	Display
	mov	es, dx
	mov	di, ax

	pop	si

	lodsw					; xDim
	stosw
	mov	dx, ax
	lodsw					; yDim
	stosw
	mul	dx				; ax contains data size
	mov	cx, 3
	rep	movsw			; xOff, yOff, skip, unused
	mov	cx, ax
	rep	movsb			; copy data

	pop	ds
	pop	es

	;show new cursor
	mov	bp, V_SHOWCUR
	call	Display

	ret

SetCursorData endp


	end

