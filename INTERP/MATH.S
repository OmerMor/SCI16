;-------------------------------------------------------------------
; MATH.S
;
; Various math routines.
;
; Modifications:  
;   04/02/90 Abdul.  Changed to use .MODEL directive and simplified
;                    segments.  Microsoft MASM 5.1.
;-------------------------------------------------------------------


;-------------------------------------------------------------------

include	macro.i
include	math.i

;-------------------------------------------------------------------

;-------------------------------------------------------------------

DosTime		equ	2ch

;-------------------------------------------------------------------

	.DATA


randSeed		word	0
RANDMASK		equ	0b400h

lcgSeed		dword	0
generator   dword	015a7c4dh
INCREMENT   equ   1

firstGuess	label	word
;		guess (hex)	range (hex)
		word	01h		;1-2
		word	02h		;2-4
		word	03h		;4-8
		word	04h		;8-10
		word	05h		;10-20
		word	07h		;20-40
		word	0ah		;40-80
		word	0eh		;80-100
		word	014h		;100-200
		word	01ch		;200-400
		word	027h		;400-800
		word	037h		;800-1000
		word	04eh		;1000-2000
		word	06fh		;2000-4000
		word	09dh		;4000-8000
		word	0deh		;8000-10000
		word	0139h		;10000-20000
		word	01bbh		;20000-40000
		word	0273h		;40000-80000
		word	0376h		;80000-100000
		word	04e6h		;100000-200000
		word	06edh		;200000-400000
		word	09cch		;400000-800000
		word	0ddbh		;800000-1000000
		word	01398h		;1000000-2000000
		word	01bb6h		;2000000-4000000
		word	02731h		;4000000-8000000
		word	0376ch		;8000000-10000000
		word	04e62h		;10000000-20000000
		word	06ed9h		;20000000-40000000
		word	09cc4h		;40000000-80000000
		word	0ffffh		;80000000-100000000


;-------------------------------------------------------------------


	.CODE


sqrt	proc	uses si, n:dword
;Return the square root of a long integer.  This uses two iterations of
;Newton-Raphson based on an educated initial guess.  In order that the
;division in N-R not overflow, the guess for the largest range is too
;high, so the value for that range will not be very accurate.
;
;N-R:
;	guess(i+1) = (guess(i) + n/guess(i))/2

	;Get the number
	mov	ax, word ptr n+_off
	mov	dx, word ptr n+_seg

	;Get the initial guess for N-R.  Shift the number right until
	;it is zero.  The number of shifts taken gives us an index
	;into firstGuess.

	.if	dx
		;Parameter is a long.  Check the high word of the number rather than the low word.
		mov	ax, dx
		.if	ah
			mov	al, ah
			mov	si, offset @data:firstGuess+(2*32)
		.else
			mov	si, offset @data:firstGuess+(2*24)
		.endif
	.else
		;See if it's a full word or a byte.
		.if	ah
			mov	al, ah
			mov	si, offset @data:firstGuess+(2*16)
		.else
			or		al, al
			jz		SqrtExit
			mov	si, offset @data:firstGuess+(2*8)
		.endif
	.endif

	mov	cx, 9
GuessLoop:
	shr	al, 1
	loopnz	GuessLoop

	;Get the index into firstGuess.
	shl	cx, 1
	sub	si, cx
	lodsw
	mov	bx, ax									;bx = guess(1)

	;Recover nl.
	mov	ax, word ptr n

	;Do first N-R.
	mov	cx, bx
	div	bx											;n/guess(1)
	xor	dx, dx									;toss remainder
	add	ax, cx
	adc	dx, 0										;(guess(1) + n/guess(1))
	shr	dx, 1
	rcr	ax, 1										;(guess(1) + n/guess(1))/2
	mov	bx, ax									;guess(2)

	;Recover number.
	mov	ax, word ptr n+_off
	mov	dx, word ptr n+_seg

	;Do second N-R.
	mov	cx, bx
	div	bx											;n/guess(2)
	xor	dx, dx									;toss remainder
	add	ax, cx
	adc	dx, 0										;(guess(2) + n/guess(2))
	shr	dx, 1
	rcr	ax, 1										;(guess(2) + n/guess(2))/2

SqrtExit:
	ret

sqrt	endp



LCGRandom	proc uses bx cx
;Linear congruential generator for random numbers.

	;Make sure we've got a non-zero seed by using the low word of the
 	;system time as the initial seed.
	mov	dx, word ptr lcgSeed
   mov   cx, word ptr lcgSeed[2]
   mov   bx,dx
   add   bx,cx
	.while !bx
		dos	DosTime
	   mov	word ptr lcgSeed, dx      ;make sure seed is current
      mov   word ptr lcgSeed[2], cx
      mov   bx,dx
      add   bx,cx
	.endw 

	mov	ax,word ptr generator
	mul	dx
   mov   bx, ax                     ;low word result
   mov   cx, dx                     ;high word result

   mov   ax, word ptr lcgSeed[2]    ;lets mult the high word seed and
   mul   word ptr generator         ;and the low word generator
   add   cx,ax                      ;add high of first to low of second mult
   adc   dx,0                       ;clear the carry into dx

   mov   ax, word ptr lcgSeed
   mul   word ptr generator[2]
   add   ax,cx
   adc   dx,0

   mov   dx,ax
   mov   ax,bx
   add   ax,INCREMENT
   adc   dx,0
           

	mov	word ptr lcgSeed, ax       ;reset the seed
   mov   word ptr lcgSeed[2], dx



  	ret

LCGRandom		endp



abs	proc	value:word

assume	ax:sword
	mov	ax, value
	.if	ax < 0
		neg	ax
	.endif
assume	ax:nothing
  	ret

abs	endp


	end

