; Pseudo Huffman compression code
; Written by Bob Heitman for Sierra On-Line, Inc.
; July 14, 1988
; This Code is (C) Copyright 1988, by Sierra On-Line, Inc.

	if1

	endif

FarPtr	struc
offw	dw	?
segw	dw	?
FarPtr	ends



_TEXT	SEGMENT  BYTE PUBLIC 'CODE'

_TEXT	ends

DGROUP	GROUP	_DATA


_DATA	segment word public 'DATA'
	ASSUME  CS: _TEXT, DS: DGROUP, SS: DGROUP, ES: DGROUP


theBit	dd	?
theByte	dw	?
byteID	db	256 dup(?)
freq	dw	256 dup(?)


; significant bit masks
masks	dw	0000000000000000b
	dw	0000000000000001b
	dw	0000000000000011b
	dw	0000000000000111b
	dw	0000000000001111b
	dw	0000000000011111b
	dw	0000000000111111b
	dw	0000000001111111b
	dw	0000000011111111b

; these values are derived from fixed table in hf.doc
MAXINDEX	equ	16	; 9 ; bytes we give special codes

;bitsNeeded	db	2,4,4,4,6,6,6,7,7, 0	; last is a dummy
bitsNeeded	db	3,4,5,5,5,5,5,5,6,6,6,6,6,6,6,6

codes	db	00b
	db	0100b
	db	00101b
	db	00110b
	db	0011100b
	db	0011101b
	db	0011110b
	db	00111110b
	db	00111111b


outBuff	dd	?

_DATA	ends


_TEXT	segment	byte public 'CODE'
	assume	cs:_TEXT, ds:DGROUP, es:DGROUP

	public	_HFEncode



_HFEncode:
; compress a near data area into a far buffer
	enter	inOff, outOff, outSeg, inSize

	; zero out our bit counter
	mov	ax, 0
	mov	word ptr theBit, ax
	mov	word ptr theBit + 2, ax
		
	; clear frequency bins and store codes
	mov	bx, 0
	mov	ax, 0
	mov	cx, 256
	do
		mov	byteID[bx], bl
		shl	bx, 1
		mov	freq[bx], 0
		shr	bx, 1
		inc	bx
	dloop

	; count frequency of bytes in data
	mov	cx, inSize
	mov	si, inOff
	do
		lodsb
		mov	bl, al
		xor	bh, bh
		shl	bx, 1
		inc	freq[bx]
	dloop

	; now we sort the bins
	do
		; set swapped flag false
		mov	dx, 0

		mov	cx, 255	; size - 1
		mov	bx, 0
		do
			shl	bx, 1
			mov	ax, freq[bx + 2]
			cmp	ax, freq[bx + 0]
			.if	greater
				xchg ax, freq[bx]
				mov  freq[bx + 2], ax

				inc  dx

				; exchange the byteID
				shr	bx, 1
				mov  al, byteID + 1[bx]
				xchg al, byteID[bx]
				mov  byteID[bx + 1], al
		
				; show a swap has occurred			
			.else
				shr	bx, 1
			.end
			inc	bx	; address next
		dloop

		; break if not swapped
		or	dx, dx
		breakif	equal
	repeat

	; now we encode the file into outOff
	mov	ax, outSeg
	mov	word ptr outBuff.segw, ax

	; write the MAXINDEX codes to buffer
	mov	ax, outOff
	mov	word ptr outBuff.offw, ax

	les	di, outBuff
	lea	si, byteID
	mov	cx, MAXINDEX
	rep	movsb

	; save DI in offset of outBuff
	mov	word ptr outBuff.offw, di

	; address input data buffer now with SI
	mov	si, inOff

	mov	di, inSize
	do

		; get a byte and check for in table
		lodsb
		xor	ah, ah
		mov	theByte, ax
		callc	InTable
		cmp	ax, MAXINDEX
		.if	equal
			; save the 8 bit code
			mov	ax, 1
			callc	PutBits, ax, ax
			callc	PutBits, theByte, #8
					
		.else
			; send the proper number of bits
			mov	bx, ax
			mov	al, codes[bx]
			mov	bl, bitsNeeded[bx]
			callc	PutBits, ax, bx
		.end

		dec	di
		breakif	equal
	repeat

	; write a terminator of right branch, most common code
	callc	PutBits, #1, #1
	mov	al, byteID
	mov	bx, 8
	callc	PutBits, ax, bx

	; return length of data (rounded up)
	mov	ax, word ptr theBit	; Get bit offset
	add	ax, 7
	mov	dx, word ptr theBit + 2
	adc	dx, 0

	; div 8 for bytes
	shr	dx, 1
	rcr	ax, 1
	shr	dx, 1
	rcr	ax, 1
	shr	dx, 1
	rcr	ax, 1

	; + MAXINDEX for common byte data
	add	ax, MAXINDEX
	exit


InTable:
; return index of this code in AX
	mov	bx, 0
	do
		cmp	al, byteID[bx]
		breakif	equal
		inc	bx
		cmp	bx, MAXINDEX
		breakif	equal
	repeat

	mov	ax, bx
	ret


PutBits:
; write the bits to the current data offset
	enter	bits, numBits

	; the arguments are unsigned characters
	and	word ptr bits, 0ffh
	and	word ptr numBits, 0ffh

 	; address the current bytes and update theBit
	mov	bx, word ptr theBit	; Get bit offset
	mov	cx, bx		; save for later
	mov	dx, word ptr theBit + 2

	; we use DI later as an index
	mov	di, numBits
	add	bx, di
	adc	dx, 0

	; we use old offset in upcoming calculations
	; and put new offset in memory location
	xchg	word ptr theBit + 2, dx
	xchg 	word ptr theBit, bx

	; develop byte offset
	shr	dx, 1
	rcr	bx, 1
	shr	dx, 1
	rcr	bx, 1
	shr	dx, 1
	rcr	bx, 1

	; find start bit position in the word starting here
	mov	dx, cx
	and	dx, 007h

	; determine shifts to align submitted bits to existing 
	; using start bit offset and number requested
	mov	cx, 16
	sub	cx, di
	sub	cx, dx
	
	; load and shift mask
	shl	di, 1
	mov	dx, masks[di]
	shl	dx, cl
	xchg	dl, dh

	; address the output buffer
	les	di, outBuff

	; clear the data (also clears bits to right)
	or	es:[di][bx], dx
	xor	es:[di][bx], dx

	; load and shift data
	mov	ax, bits
	shl	ax, cl
	xchg	al, ah

	; or in the data
	or	es:[di][bx], ax

	; we are done
	exit


_TEXT	ends

	END

