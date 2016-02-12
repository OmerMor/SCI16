;-------------------------------------------------------------------
; hfd.s
; Pruned Huffman de-compression code
; Written by Bob Heitman for Sierra On-Line, Inc.
; July 14, 1988
; This Code is (C) Copyright 1988, by Sierra On-Line, Inc.
; Modifications:  
;   3/12/90 Abdul.  Changed to use .MODEL directive and simplified
;                   segments.  Microsoft MASM 5.1.
;-------------------------------------------------------------------


	include macro.i
	include fileio.i
	include memmgr.i
	include resource.i
	include io.i
	include stdio.i


Decode	proto	near
GetBits	proto	near	numBits:word

BUFFSIZE		equ	1024							; working input buffer size
OVERRUN		equ	3								; overhead for 12 bit read
HEADERSIZE	equ	4



	.DATA


Node	struc
	code	byte	?
	kids	byte	?
Node	ends

theBit		word	?
iHandle		word	?
inBuff		dword	?
nodeHandle	word	?
nodeBuff		dword	?
outBuff		dword	?
fileHandle	word	?
numNodes		byte	?
eofCode		byte	?	; most common code from header



	even

; significant bit masks
masks	word	0000000000000000b
		word	0000000000000001b
		word	0000000000000011b
		word	0000000000000111b
		word	0000000000001111b
		word	0000000000011111b
		word	0000000000111111b
		word	0000000001111111b
		word	0000000011111111b



	.CODE 

; Tree header on compressed file is as follows:
;	byte	number of nodes in tree (n)
;	byte	most common code (used as EOF when in right branch)
;	; for each node in table...
;	byte	value of external node (undefined for internal)
;	byte	nibble offsets to kids (HI = left/LO = right), 00 = end
;	.
;	.
;



HFDecode proc uses si di, fd:word, oHandle:word
; passed an input buffer and an output buffer
; decode and return length

	mov	ax, fd
	mov	fileHandle, ax

	; read number of nodes and eof code
	invoke	read, fileHandle, addr numNodes, 2	; also fills eofCode

	; determine size of node header and get handle to hunk
	mov	al, numNodes
	xor	ah, ah
	shl	ax, 1
	push	ax
	invoke	GetResHandle, ax
	mov	nodeHandle, ax
	pop	cx

	; read header into nodeHandle
	invoke	readfar, fileHandle, nodeHandle, cx

	; get a work buffer
	invoke	GetResHandle, BUFFSIZE+OVERRUN
	mov	iHandle, ax

	; dereference output handle
	mov	si, oHandle
	mov	ax, [si]
	mov	word ptr outBuff,ax
	mov	ax, [si + 2]		;Get segment address of output buffer
	mov	word ptr outBuff + 2,ax

	; dereference node handle
	mov	si, nodeHandle
	mov	ax, [si]
	mov	word ptr nodeBuff,ax
	mov	ax, [si + 2]
	mov	word ptr nodeBuff + 2,ax

	; dereference working buffer handle
	mov	si, iHandle
	mov	ax, [si]
	mov	word ptr inBuff,ax
	mov	ax, [si + 2]
	mov	word ptr inBuff + 2,ax

	; initialize theBit to force first read
	mov	theBit, BUFFSIZE*8

	; call decode until it returns EOF
	.while TRUE
		invoke	Decode

		; EOF is most common code in right branch (in header)
		.break .if ah && (al == eofCode)

		; store the byte in AL
		les	di, outBuff
		stosb		
		inc	word ptr outBuff
		inc	si
	.endw

	; release our buffers
	invoke	DisposeHandle, iHandle
	invoke	DisposeHandle, nodeHandle

	ret

HFDecode endp



Decode proc near uses si
; return one full byte from input data

assume	si:ptr Node

	les	si, nodeBuff

	.while TRUE
		; check for complete (at an external node)
		mov	al, es:[si].kids
		.if	!al
			xor	ah, ah
			mov	al, es:[si].code
			.break
		.endif

		; get the next bit from the stream
		push	es
		invoke	GetBits, 1
		pop	es

		; advance to next node
		or		al, al
		mov	al, es:[si].kids	; DOESN'T AFFECT FLAGS
		.if	zero?
			; left branch
			; use HI nibble only
			and	ax, 000f0h

			; word size table
			shr	al, 1
			shr	al, 1
			shr	al, 1
		.else
			; right branch (may have exit)
			; use LOW nibble only
			and	ax, 0000fh
			.if	zero?
				; 9 bit escapement
				invoke	GetBits, 8
				or		ah, 01h							; flag as literal byte
				.break
			.endif

			; word size table
			shl	al, 1
		.endif

		; add this offset to current node pointer
		add	si, ax
	.endw

	ret

assume	si:nothing

Decode endp



GetBits proc near uses si di, numBits:word
; return in AX the requested number of bits from "theBit" in data

	; check for exhausted buffer
	mov	bx, theBit
	.if	bx >= BUFFSIZE*8
		sub	bx, BUFFSIZE*8
		mov	theBit, bx

		; read BUFFSIZE + OVERRUN into buffer
		invoke	readfar, fileHandle, iHandle, BUFFSIZE+OVERRUN

		; now seek back 3 bytes for next read
		invoke	lseek, fileHandle, -OVERRUN, 1

		; reload BX
		mov	bx, theBit
	.endif

	; address the current bytes and update theBit
	mov	cx, bx		; save for later

	; we use DI later as an index
	mov	di, numBits
	add	bx, di

	; we use old offset in upcoming calculations
	; and put new offset in memory location
	xchg 	theBit, bx

	; develop byte offset
	shr	bx, 1
	shr	bx, 1
	shr	bx, 1

	; find start bit position in the word starting here
	mov	dx, cx
	and	dx, 007h

	; determine shifts to "right justify" requested bits 
	; using start bit offset and number requested
	mov	cx, 16
	sub	cx, di
	sub	cx, dx

	; load and shift the next two bytes from data
	les	si, inBuff
	mov	ax, es:[si][bx]
	xchg	al, ah
	shr	ax, cl

	; mask off upper bits of return
	shl	di, 1
	and	ax, masks[di]

	; return the value
	ret

GetBits endp

	END

