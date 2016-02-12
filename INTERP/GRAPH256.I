;-------------------------------------------------------------------
;	graph256.i
;
;	Header for 256 color graphics routines
;-------------------------------------------------------------------


ifndef GRAPHxxx_I

GRAPHxxx_I	equ	1


externdef	pcSeg: word
externdef	map16to256: byte
	
FastMatch	proto	palOff:word, palSeg:word, redV:word, greenV:word, blueV:word, psize:word, least:word

MagCursorStruct	struc
	pictDataOff			dw ?
	pictDataSeg			dw ?
	cursorDataOff		dw ?
	cursorDataSeg		dw ?
	replaceColor		db ?
	curTop				dw ?
	curLeft				dw ?
	curBottom			dw ?
	curRight				dw ?	
	picXDim				dw	?
	magLevel				dw ?
MagCursorStruct	ends

pseudoCursor		struc
	xDim					dw ?
	yDim					dw ?
	xHot					dw ?
	yHot					dw ?
	skipColor			db ?
pseudoCursor 		ends




;;;;; Macros

SetVNib	macro
	mov	al, vColor
	endm


ShiftVNib	macro
	endm

SetBase			macro
;Calculate base address of startY
	mov	di, penY
	shl	di, 1
	mov	di, baseTable[di]
	endm

SetFillBase			macro
;Calculate base address of startY
	mov	si, penY
	shl	si, 1
	mov	si, baseTable[si]
	endm


FastDot	macro
;Quick draw of points of constant color at the same y coordinate.
;Must call SetFastDot first, then preserve the di from that to use
;this macro.

;Registers trashed: ax, bx, si

	push	cx

	; adword in offset of startX 
	mov	bx, penX
	lea	di, [bx][si]							; points at byte containing point

	; now plot each active bitMap
	mov	bx, mapSet
	.if	bx & VMAP
		mov	es, vSeg
		mov	al, vColor							; color	0-256 in byte
		mov	es:[di], al
	.endif

	.if	bx & PMAP
		mov	es, pcSeg
		and	byte ptr es:[di], EVENOFF
		mov	al, pColor							; 0-F in hi nibble only
		or		es:[di], al
	.endif

	.if	bx & CMAP
		mov	es, pcSeg
		and	byte ptr es:[di], ODDOFF
		mov	al, cColor							; 0-F in low nibble
		or		es:[di], al
	.endif

	pop	cx

	endm


PlotDot	macro
;Plot point macro.
	SetVNib
	SetBase
	FastDot
	endm



CheckDot		macro
	; Macro to check for an unfilled point in Fill
	; Using:	BX = penX, SI base address of line
	; Returns: 	Z set if point is unfilled
	; Destroys:	AX

pushcontext	all
assume	bx:sword

	;Check for a dot out of map coords
	.if	(bx < theRect.left) || (bx >= theRect.right)
		or		al, 0ffh
	.else
		.if		mapSet & VMAP
			mov	es, vSeg
			mov	al, es:[si][bx]
			cmp	al, vWHITE
		.else
			mov	es, pcSeg
			.if	mapSet & PMAP
				test	byte ptr es:[si][bx], EVENON
			.else
				test	byte ptr es:[si][bx], ODDON
			.endif
		.endif
	.endif

popcontext	all

	endm

Base	macro	reg, table
	shl	reg, 1
	mov	reg, table[reg]
	endm

MirrorX	macro	reg   ;x coord in ax
 	sub   reg,319
	neg   reg
	endm



REPEATC			equ	080h
REPSKIP			equ	040h
UNIQUE			equ	000h

VROWBYTES		equ	320	; bytes per scanline

PAL_FILE_SIZE	equ	504h
PAL_CLUT_SIZE	equ	256
PAL_MATCH		equ	1
PAL_REPLACE		equ	2


Guns	struc
	flags		byte	?
	r			byte	?
	g			byte	?
	b			byte	?
Guns	ends
	
RPalette	struc
	mapTo				byte	256  dup(?)
	valid				dword	?
	gun				byte	1024 dup(?)
	palIntensity	word	256  dup(?)
RPalette	ends


endif

