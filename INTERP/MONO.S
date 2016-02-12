;---------------------------------------------------------------------
; MONO.S
; Copies text strings to the graphics text page
;---------------------------------------------------------------------

include	macro.i
include	mono.i

	.DATA

MONO_CRTC_ADDR_REG	equ	3b4h
MONO_CRTC_DATA_REG	equ	3b5h
CURSOR_LOC_REG			equ	0fh

TEXT1			equ	0b000h
TEXT2			equ	0b800h

monoInited	byte	0
xlate			word	0
dseg			word	TEXT1
doff			word	0
	.CODE


DoXlate	proc

	.if !monoInited
		call	MonoInit
	.endif
	.if monoInited != 1
		ret
	.endif
	call	MonoClear		; clear mono screen
	mov	xlate, 1			; allow foreign language translations to mono mon.
	ret

DoXlate	endp

MonoStr	proc uses si di es, tOffset:word, tSegment:word

	.if !monoInited
		call	MonoInit
	.endif
	.if monoInited != 1
		ret
	.endif
	mov	si, tOffset
	mov	ax, tSegment
	.if	(si == 0) && (ax == 0)
		call	MonoClear
		ret
	.endif
	.if	(si == 1) && (ax == 0)
		mov	dseg, TEXT1
		ret
	.endif
	.if	(si == 2) && (ax == 0)
		mov	dseg, TEXT2
		ret
	.endif
	mov	es, dseg
	mov	di, doff
	push	ds
	mov	ds, ax
ms1:
	lodsb
	or		al, al
	jz		ms2
	stosb
	inc	di
	cmp	di, 4000
	jl		ms1
	pop	ax
	push	ds
	mov	ds, ax
	call	MonoScroll
	mov	di, doff
	pop	ax
	push	ds
	mov	ds, ax
	jmp	ms1
ms2:
	pop	ds
	mov	doff, di
	call	MonoNewline
	ret

MonoStr	endp

MonoInit	proc	uses dx

	; detect presence of mono monitor
	mov	dx, MONO_CRTC_ADDR_REG
	mov	al, CURSOR_LOC_REG
	out	dx, al			; access cursor loc reg
	mov	dx, MONO_CRTC_DATA_REG
	in		al, dx
	inc	al					
	out	dx, al			; attempt to modify it
	mov	ah, al
	in		al, dx
	.if al == ah
		dec	al				; it's modifiable; ergo mono monitor exists
		out	dx, al		; (clean-up mess)
		mov	monoInited, 1
	.else
		mov	monoInited, -1
	.endif
	ret

MonoInit	endp

MonoClear proc uses di cx es

	mov	es, dseg
	xor	di, di
	mov	doff, di
	mov	ax, 720h
	mov	cx, 4000
	rep	stosw
	ret

MonoClear endp

MonoNewline proc uses cx dx

	mov	ax, doff
	mov	cx, 160
	xor	dx, dx
	div	cx
	mul	cx
	add	ax, cx
	mov	doff, ax
	.if ax >= 4000
		call	MonoScroll
	.endif
	ret

MonoNewline endp

MonoScroll proc uses si di 

	push	ds
	push	es
	mov	ax, dseg
	mov	ds, ax
	mov	es, ax
	mov	si, 160
	xor	di, di
ss1:
	lodsb
	inc	si
	stosb
	inc	di
	cmp	si, 4000
	jl		ss1
	mov	si, di
	mov	al, ' '
ss2:
	stosb
	inc	di
	cmp	di, 4000
	jl		ss2
	pop	es
	pop	ds
	mov	doff, si
	ret

MonoScroll endp

	end

