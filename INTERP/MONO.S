;---------------------------------------------------------------------
; MONO.S
; Copies text strings to the graphics text page
;---------------------------------------------------------------------

include	macro.i
include	mono.i

	.DATA

TEXT1			equ	0b000h
TEXT2			equ	0b800h

xlate			word	0
dseg			word	TEXT1
doff			word	0

	.CODE


DoXlate	proc

	call	DClear
	mov	xlate, 1
	ret

DoXlate	endp

MonoStr	proc uses si di es, tOffset:word, tSegment:word

	mov	si, tOffset
	mov	ax, tSegment
	.if	(si == 0) && (ax == 0)
		call	DClear
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
fb1:
	lodsb
	or		al, al
	jz		fb2
	stosb
	inc	di
	cmp	di, 4000
	jl		fb1
	pop	ax
	push	ds
	mov	ds, ax
	call	DScroll
	mov	di, doff
	pop	ax
	push	ds
	mov	ds, ax
	jmp	fb1
fb2:
	pop	ds
	mov	doff, di
	call	Dnl
	ret

MonoStr	endp

DClear proc uses di cx es

	mov	es, dseg
	xor	di, di
	mov	doff, di
	mov	ax, 720h
	mov	cx, 4000
	rep	stosw
	ret

DClear endp

Dnl proc uses cx dx

	mov	ax, doff
	mov	cx, 160
	xor	dx, dx
	div	cx
	mul	cx
	add	ax, cx
	mov	doff, ax
	cmp	ax, 4000
	jl		dnl1
	call	DScroll
dnl1:
	ret

Dnl endp

DScroll proc uses si di 

	push	ds
	push	es
	mov	ax, dseg
	mov	ds, ax
	mov	es, ax
	mov	si, 160
	xor	di, di
dscrl:
	lodsb
	inc	si
	stosb
	inc	di
	cmp	si, 4000
	jl		dscrl
	mov	si, di
	mov	al, ' '
dscrl2:
	stosb
	inc	di
	cmp	di, 4000
	jl		dscrl2
	pop	es
	pop	ds
	mov	doff, si
	ret

DScroll endp

	end

