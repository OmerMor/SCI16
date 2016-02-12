;---------------------------------------------------------------------------
; extmem.s		Mark Wilden, January 1991
;
; routines for accessing extended memory from the alternate memory manager
; see Ray Duncan, Extending DOS, for details
;---------------------------------------------------------------------------

include	macro.i

include	armasm.i
include	dialog.i
include	errmsg.i
include	extmem.i
include	stdio.i

; DOS functions
DOS_GETVECT			equ 35h

; Interrupt vectors
BOOTSTRAP_INT		equ 19h

; Heuristic (though conservative) guesses
LOWEST_DEVICEDRIVER_SEG	equ	0500h
ROM_BIOS_SEG				equ	0f000h

; Extended memory functions
EXTMEM_INT			equ 15h

EXTMEM_MEMCPY		equ 87h
EXTMEM_MEMAVAIL	equ 88h

; protected mode data structures
descriptor struc
	segmentLen		word	0
	address			byte	3 dup (0)
	accessRights	byte	0
	reserved			byte	2 dup (0)
descriptor ends

.data

gdt label byte	; would prefer to use a struc, but MASM won't nest them
dummy				descriptor <>
romBiosData		descriptor <>
source			descriptor <>
dest				descriptor <>
romBiosCode		descriptor <>
romBiosStack	descriptor <>

extMem ARMType <	\
	extMemName,		\
	'+',				\
	init,				\
	memcpy,			\
	term,				\	
	PAGESIZE			\
>

extMemName	byte	"extended", 0
msgBufLen	equ $ - extMemName
msgBuf		byte	msgBufLen dup (?)

.code

;---------------------------------------------------------------------------
; init	determine if extended memory is available, and if so, how much.
;			make sure that no one else is using any of it
;
; entry:	nothing
; exit:	ax	= success
;---------------------------------------------------------------------------
init	proc private uses si di es ds

; see if we're even on a 80286 or better machine by seeing what goes
; on the stack on a PUSH SP
	push	sp
	pop	ax
	cmp	sp, ax
	jne	noExtMem
	
; examine the two interrupt vectors that users of extended memory generally
; hook into.  if either of the vectors seem to be hooked, bag it.
	dos	DOS_GETVECT, BOOTSTRAP_INT
	mov	ax, es
	.if	ax >= LOWEST_DEVICEDRIVER_SEG
		cmp	ax, ROM_BIOS_SEG
		jb		noExtMem
	.endif
	
	dos	DOS_GETVECT, EXTMEM_INT
	mov	ax, es
	cmp	ax, LOWEST_DEVICEDRIVER_SEG
	.if	ax >= LOWEST_DEVICEDRIVER_SEG
		cmp	ax, ROM_BIOS_SEG
		jb		noExtMem
	.endif

; see how much extended memory is available
	xor	al, al
	mov	ah, EXTMEM_MEMAVAIL
	int	EXTMEM_INT				; returns ax = K of extended memory
	jc		noExtMem

	mov	cl, PAGESIZE_LOG2 - 10
	shr	ax, cl
	.if	ax > MAXPAGES
		mov ax, MAXPAGES
	.endif
	mov	extMem.nPages, ax
	mov	extMem.nFreePages, ax

; do a test copy to be sure that extended memory works, just in case we're
; on an 80186 
	push	ds
	pop	es

; push the count
	mov	cx, msgBufLen	
	push	cx
	
; push the source address
	lea	di, extMemName
	call	segAdr2Linear
	push	ax
	push	dx

; push the destination address
	lea	di, msgBuf
	call	segAdr2Linear
	push	ax
	push	dx

; and let her rip
	call	linearMemCpy	
	add	sp, 10
	or		ah, ah
	jne	noExtMem
	
; compare to see that the copy worked
	lea	si, msgBuf
	lea	di, extMemName
	mov	cx, msgBufLen	
	cld
	repe	cmpsb
	jne	noExtMem

; allocate the pageMap
	invoke ARMAllocPageMap, addr extMem
	jmp	initEnd

noExtMem:
	mov	extMem.nFreePages, 0
	mov	extMem.nPages, 0

initEnd:
	mov	ax, extMem.nFreePages
	mov	extMem.active, ax

	ret

init	endp

;---------------------------------------------------------------------------
; memcpy		moves data from memory to/from extended memory page
;
; entry:	es:di = data
;			ax		= direction (PAGE2MEM or MEM2PAGE)
;			bx		= page
;			cx		= count
;			dx		= offset into page
; exit:	ah		= 0 if success
;---------------------------------------------------------------------------
memcpy proc private	uses bx cx dx si di

	mov	si, ax

; push the count
	push	cx
	
	mov	cx, dx

; convert the segmented address and the page to linear addresses

; jump around according to direction we're copying
	cmp	si, PAGE2MEM
	je		pushPage

pushSegAdr:
	call	segAdr2Linear
	push	ax
	push	dx
	cmp	si, PAGE2MEM
	je		doCopy

pushPage:
	mov	dx, cx
	call	page2Linear
	push	ax
	push	dx
	cmp	si, PAGE2MEM
	je		pushSegAdr

doCopy:
	call	linearMemCpy
	add	sp, 10

	.if ah
		mov	al, ah
		xor	ah, ah
		invoke	RAlert, E_EXT_MEM, ax
		mov	ah, 1
	.endif

	ret

memcpy endp

;---------------------------------------------------------------------------
; term	free the page map and deactivate
;
; entry:	nothing
; exit:	nothing
;---------------------------------------------------------------------------
term	proc private

	invoke ARMFreePageMap, addr extMem
	mov	extMem.active, FALSE

	ret

term	endp

;---------------------------------------------------------------------------
; linearMemCpy		copies to/from linearly addressed memory (16M adress space)
;
; entry:	aSource	= linear source address
; 			aDest		= linear destination address
;
; exit:	ah			= 0 if success, else error code
;---------------------------------------------------------------------------
linearMemCpy	 proc near private uses cx si es, \
													destHi:byte, destLo,	\
								 					srcHi:byte, srcLo, count
	mov	cx, count

	mov	dest.accessRights, 93h
	mov	ax, destLo
	mov	word ptr dest.address, ax
	mov	al, destHi
	mov	dest.address[2], al
	mov	dest.segmentLen, cx

	mov	source.accessRights, 93h
	mov	ax, srcLo
	mov	word ptr source.address, ax
	mov	al, srcHi
	mov	source.address[2], al
	mov	source.segmentLen, cx
	
	push	ds
	pop	es
	lea	si, gdt
	shr	cx, 1
	adc	cx, 0
	mov	ah, EXTMEM_MEMCPY
	int	EXTMEM_INT

	; if carry, but ah is 0 (for whatever warped reason), set ah to -1
	; to make sure the error is observed
	.if 	carry?
		.if	!ah
			mov	ah, 0ffh
		.endif
	.else
		xor	ah, ah
	.endif

	ret

linearMemCpy	endp

;---------------------------------------------------------------------------
; page2Linear	converts a page number to a linear address
;
; entry:	bx		= page number
;			dx		= offset into page
; exit:	dl:ax	= linear address
;---------------------------------------------------------------------------
page2Linear	proc near private uses cx

	mov	cx, dx
	mov	ax, bx
	shr	ax, 1						; convert from offset to index
	mov	dx, PAGESIZE
	mul	dx							; dl:ax = offset into extended memory
	add	ax, cx
	adc	dx, 0						
	add	dl, 1 shl (20 - 16)	; add 1M to form linear address in dl:ax
	ret

page2Linear	endp

;---------------------------------------------------------------------------
; segAdr2Linear	converts a segmented address to a linear address
;
; entry:	es:di	= segmented address
; exit:	dl:ax	= linear address
;---------------------------------------------------------------------------
segAdr2Linear	proc near private

	mov	ax, es
	mov	dx, 16
	mul	dx							; dx:ax = segment converted to linear
	add	ax, di					; add the offset to the linear
	adc	dx, 0
	ret

segAdr2Linear endp

	end
