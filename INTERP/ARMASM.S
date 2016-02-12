;---------------------------------------------------------------------------
; armasm.s		Mark Wilden, May 1992
;
; assembly helper routines for alternate resource memory access
;---------------------------------------------------------------------------

include	macro.i

include	armasm.i
include	memmgr.i

FREE_PAGE			equ -1

.data

inARM		word	FALSE

.code

getFreePage	proto near	arm:ptr ARMType

;---------------------------------------------------------------------------
; ARMAllocPageMap		allocate page map
;
; entry:	arm	= pointer to ARMType
; exit:	nothing
;
; all registers are saved because this routine is called from assembly and
; makes C calls
;---------------------------------------------------------------------------
ARMAllocPageMap	proc uses bx cx dx si di es, arm:ptr ARMType

	mov	si, arm
	assume si:ptr ARMType
	mov	ax, [si].nPages
	.if ax
		shl	ax, 1					; convert number of pages to number of bytes
		invoke NeedHandle, ax
		mov	[si].pageMap, ax
		mov	bx, FREE_PAGE
		invoke FillHandle, ax, bx
	.endif

	assume si:nothing
	ret

ARMAllocPageMap	endp

;---------------------------------------------------------------------------
; getPageMap	get an entry in the pageMap
;
; entry:	ofs	= byte offset in map
;			si		= ptr to arm
; exit:	bx		= entry
;---------------------------------------------------------------------------
getPageMap	macro ofs
	push	ds
	push	di

	mov	di, [si].pageMap
	lds	di, [di]
	add	di, ofs
	mov	bx, [di]

	pop	di
	pop	ds
endm

;---------------------------------------------------------------------------
; setPageMap	set an entry in the pageMap to a value
;
; entry:	ofs		= byte offset to set
;			val		= word value
;			si			= ptr to arm
; exit:	nothing
;---------------------------------------------------------------------------
setPageMap	macro ofs, val
	push	ds
	push	di

	mov	di, [si].pageMap
	lds	di, [di]
	add	di, ofs
	mov	[di], val

	pop	di
	pop	ds
endm

;---------------------------------------------------------------------------
; bytes2NPages		determine number of pages for this number of bytes
;
; entry:	count	= number of bytes
; exit:	cx		= number of pages
;---------------------------------------------------------------------------
bytes2NPages macro count
	push	ax
	mov	ax, count
	add	ax, PAGESIZE - 1			; round up
	.if carry?							; make sure rounding didn't overflow
		mov	ax, MAXPAGES_REQUEST
	.else
		mov	cl, PAGESIZE_LOG2
		shr	ax, cl
	.endif
	mov	cx, ax						; cx = number of pages to write
   pop	ax
endm

;---------------------------------------------------------------------------
; ARMAlloc	allocate a set of ARM pages
;
; entry:	arm	= pointer to ARMType
;			count	= number of bytes of data
; exit:	ax		= ARM handle, else NO_MEMORY if no memory available
;---------------------------------------------------------------------------
ARMAlloc proc 	uses si 	arm:ptr ARMType, count: word
	local firstPage: word
	local prevPage: word

	mov	si, arm
	assume si:ptr ARMType

   bytes2NPages	count				; cx = number of pages to write

	; make sure we have enough memory for this request
	.if cx > [si].nFreePages
		mov	ax, NO_MEMORY
		jmp	@F
	.endif

	; loop through the number of pages we're allocating, and link each into
	; our pageMap

	xor	dx, dx						; dx = physical page

	.repeat
		invoke getFreePage, arm		; bx = byte offset into pageMap of free
											; page, or NO_MEMORY if none
		.if bx == NO_MEMORY
			mov	ax, NO_MEMORY
			jmp	@F
		.endif

		; if this is the first page allocated of this request, save its byte
		; offset into pageMap to return to caller as the ARM logical handle
		.if !dx
			mov	firstPage, bx

		; otherwise, update the link of the previous page
		.else
			setPageMap prevPage, bx
		.endif

		; make current link point to us, indicating end of chain
		setPageMap bx, bx

		; update amount of pages free
		dec	[si].nFreePages

		; remember this page
		mov	prevPage, bx

		; and move on to next physical page
		inc	dx

	.untilcxz

	mov	ax, firstPage

@@:
	assume si:nothing
	ret

ARMAlloc	endp

;---------------------------------------------------------------------------
; ARMCopy	read or write data in a set of ARM pages
;
; entry:	direction	= to or from ARM page (PAGE2MEM or MEM2PAGE)
;			arm			= pointer to ARMType
;			page			= ARM page
;			offset		= where in ARM allocation to copy
;			count			= number of bytes of data
;			data			= far pointer to data
; exit:	ax				= 0, or NO_MEMORY if error
;---------------------------------------------------------------------------
ARMCopy proc uses es si di, direction: word, arm:ptr ARMType, aPage: word, \
									 aOffset: word, count: word, data: far ptr
	mov	inARM, TRUE

	mov	si, arm
	assume si:ptr ARMType

	mov	bx, aPage			; bx = current page
   mov	cx, count			; cx = bytes left to copy
	mov	dx, aOffset			; dx = offset
	les	di, data				; di = data

	.repeat
		.if dx >= PAGESIZE
			; skip this page, since beginning of request is in a later page
			sub	dx, PAGESIZE

		.else
			push	cx

			; determine how many bytes of this page to copy
			.if cx > PAGESIZE
				mov	cx, PAGESIZE
				sub	cx, dx
			.endif
			
			; copy the data
         push	cx
			mov	ax, direction
			call	[si].memcpy
         pop	cx

			mov	dx, cx				; dx = number of bytes copied
			pop	cx

			.if ah
				mov	ax, NO_MEMORY
				jmp	@F
			.endif
			
			; bump down number of bytes--bump up pointer to data
			sub	cx, dx
			add	di, dx

			; all other copies will start at bottom of page
			mov	dx, 0					; dx = offset (0)
			
		.endif

		getPageMap	bx

	.until !cx
		
	mov	ax, 0

@@:
	assume si:nothing
	
	mov	inARM, FALSE

	ret

ARMCopy	endp

;---------------------------------------------------------------------------
; ARMFree	release ARM memory
;
; entry:	arm	= ptr to ARMType
;			aPage	= logical handle
; exit:	nothing
;---------------------------------------------------------------------------
ARMFree	proc uses ax bx si di es, arm:ptr ARMType, thePage: word

	; loop through pageMap while there is a link to another entry, setting the
	; entry to FREE_PAGE and incrementing # of free pages

	mov	bx, thePage
	mov	si, arm
	assume si:ptr ARMType
	mov	di, [si].pageMap
	les	di, [di]
	.repeat
		mov	ax, es:[di][bx]
		mov	word ptr es:[di][bx], FREE_PAGE
		inc	[si].nFreePages
		cmp	ax, bx
		mov	bx, ax
	.until zero?

	assume si:nothing
	ret

ARMFree	endp

;---------------------------------------------------------------------------
; ARMMemAvail	return amount of memory free
;
; entry:	nothing
; exit:	dx:ax	= number of bytes free (long value)
;---------------------------------------------------------------------------
ARMMemAvail	proc	uses si, arm:ptr ARMType

	mov	ax, PAGESIZE
	mov	si, arm
	mul	(ARMType ptr [si]).nFreePages

	ret

ARMMemAvail	endp

;---------------------------------------------------------------------------
; ARMCritical		return whether currently executing ARM copy
;
; entry:	nothing
; exit:	ax	= currently executing ARM copy
;---------------------------------------------------------------------------
ARMCritical proc

	mov	ax, inARM
	ret

ARMCritical	endp

;---------------------------------------------------------------------------
; ARMFreePageMap		free page map
;
; entry:	arm	= pointer to ARMType
; exit:	nothing
;---------------------------------------------------------------------------
ARMFreePageMap	proc uses ax bx cx dx si di es, arm:ptr ARMType

	mov	si, arm
	assume si:ptr ARMType
	.if [si].pageMap
		invoke DisposeHandle, [si].pageMap
	.endif

	assume si:nothing
	ret

ARMFreePageMap	endp

;---------------------------------------------------------------------------
; getFreePage	find a free page in the pageMap
;
; entry:	arm	= pointer to ARMType
; exit:	bx		= byte offset to free page in pageMap, or NO_MEMORY if none
;---------------------------------------------------------------------------
getFreePage	proc near private uses cx si ds, arm:ptr ARMType

	mov	si, arm
	assume si:ptr ARMType
	mov	cx, [si].nPages
	mov	bx, [si].pageMap
	lds	bx, [bx]

	mov	si, 0
	.repeat
		.break .if word ptr [bx+si] == FREE_PAGE
		add	si, 2
	.untilcxz
	.if	!cx
		mov	bx, NO_MEMORY
	.else
		mov	bx, si
	.endif

	assume si:nothing
	ret

getFreePage	endp

	end

