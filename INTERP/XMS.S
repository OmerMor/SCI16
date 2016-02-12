;---------------------------------------------------------------------------
; xms.s		Mark Wilden, January 1991
;
; routines for accessing XMS memory from the alternate memory manager
; see Ray Duncan, Extending DOS, for details
;---------------------------------------------------------------------------

include	macro.i

include	armasm.i
include	errmsg.i
include	start.i
include	stdio.i
include	xms.i

; Interrupt vectors
MULTIPLEX_INT		equ 2Fh

; Xtended Memory Manager functions
XMM_MULTIPLEX_FUNC			equ 43h
XMM_MULTIPLEX_CHKSTAT		equ 00h
XMM_MULTIPLEX_INSTALLED		equ 80h
XMM_MULTIPLEX_GETENTRYPT	equ 10h

XMM_MEMAVAIL	equ 08h
XMM_ALLOC		equ 09h
XMM_RELEASE		equ 0Ah
XMM_MEMCPY		equ 0Bh

.data

xms	ARMType	<	\
	xmsName,			\
	'%',				\
	init,				\
	memcpy,			\
	term,				\
	PAGESIZE			\
>

xmmEntry		dword	?
xmmHandle	word	?
xmsName		byte	"XMS", 0

xmmParmBlock struc
	copyLength	dword	0
	srcHandle	word	0	; xmmHandle or 0 if src is standard memory
	srcAddress	dword	0	; offset into allocated XMS memory, or far adr if src
	dstHandle	word	0	;	is standard memory
	dstAddress	dword	0
xmmParmBlock ends

xmmParms		xmmParmBlock <>

.code

;---------------------------------------------------------------------------
; init	determine if XMS memory is available, and if so, how much.
;			allocate it all
;
; entry:	nothing
; exit:	ax	= success
;---------------------------------------------------------------------------
init	proc private uses es

	mov	xms.nPages, 0
	mov	xms.nFreePages, 0

	; see if an XMM is installed
	mov	ah, XMM_MULTIPLEX_FUNC
	mov	al, XMM_MULTIPLEX_CHKSTAT
	int	MULTIPLEX_INT
	.if	al == XMM_MULTIPLEX_INSTALLED
		; obtain entry point to the XMM
		mov	ah, XMM_MULTIPLEX_FUNC
		mov	al, XMM_MULTIPLEX_GETENTRYPT
		int	MULTIPLEX_INT		; es:bx = entry point
		mov	word ptr xmmEntry[_off], bx
		mov	word ptr xmmEntry[_seg], es
	
		; determine number of pages available
		mov	ah, XMM_MEMAVAIL
		call	xmmEntry				; ax = largest block (K), dx = total (K)
		mov	cl, PAGESIZE_LOG2 - 10
		shr	ax, cl
		.if	ax > MAXPAGES
			mov ax, MAXPAGES
		.endif
		.if	ax >= MINPAGES
			mov	xms.nPages, ax
			mov	xms.nFreePages, ax
			invoke ARMAllocPageMap, addr xms

			; grab them
			mov	ah, XMM_ALLOC
			mov	dx, xms.nFreePages
			shl	dx, cl				; convert from 16K back to K
			call	xmmEntry				; dx = handle
			.if	ax
				mov	xmmHandle, dx
			.else
				mov	xms.nPages, 0
				mov	xms.nFreePages, 0
			.endif
		.endif
	.endif

	mov	ax, xms.nFreePages
	mov	xms.active, ax

	ret

init	endp

;---------------------------------------------------------------------------
; memcpy		moves data from memory to/from XMS memory page
;
; entry:	es:di = data
;			ax		= direction (PAGE2MEM or MEM2PAGE)
;			bx		= page
;			cx		= count
;			dx		= offset into page
; exit:	ah		= 0 if success
;---------------------------------------------------------------------------
memcpy proc private	uses ax dx si

	mov	si, ax

	push	si											; push the direction
	push	cx											; push the count

	; jump around according to direction we're copying
	cmp	si, PAGE2MEM
	je		pushPage

pushSegAdr:
	mov	ax, 0				; "handle" indicating this operand is std memory
	push	ax
	push	di
	push	es
	cmp	si, PAGE2MEM
	je		doCopy

pushPage:
	mov	ax, xmmHandle
	push	ax
	call	page2XMSOfs
	push	ax
	push	dx
	cmp	si, PAGE2MEM
	je		pushSegAdr

doCopy:
	call	xmmMemCpy
	add	sp, 16
	
	.if ax == 1
		mov ah, 0
	.else
		mov ah, 1
	.endif

	ret

memcpy endp

;---------------------------------------------------------------------------
; term	deallocate the memory we got
;
; entry:	nothing
; exit:	nothing
;---------------------------------------------------------------------------
term	proc private

	mov	ah, XMM_RELEASE
	mov	dx, xmmHandle
	call	xmmEntry

	invoke ARMFreePageMap, addr xms
	mov	xms.active, FALSE

	ret

term	endp

;---------------------------------------------------------------------------
; xmmMemCpy		copies to/from XMS memory
;
; entry:	dstHi:dstLo	=	destination XMS offset or far address
;			dstHandle	=	XMS handle or 0, if dest is standard memory
; 			srcHi:srcLo
;			srcHandle
; 			count			= 16-bit byte count (though XMS can handle 32-bit counts)
;			dir			= PAGE2MEM or MEM2PAGE (for error message only)
;
; exit:	ax				= 1 if success
;---------------------------------------------------------------------------
xmmMemCpy	 proc near private uses es bx cx dx si di, \
				 destHi:word, destLo:word, destHandle:word, \
				 sourceHi:word, sourceLo:word, sourceHandle:word,	\
				 count:word, dir:word

	mov	ax, destLo
	mov	word ptr xmmParms.dstAddress[_off], ax
	mov	ax, destHi
	mov	word ptr xmmParms.dstAddress[_seg], ax
	mov	ax, destHandle
	mov	xmmParms.dstHandle, ax

	mov	ax, sourceLo
	mov	word ptr xmmParms.srcAddress[_off], ax
	mov	ax, sourceHi
	mov	word ptr xmmParms.srcAddress[_seg], ax
	mov	ax, sourceHandle
	mov	xmmParms.srcHandle, ax

	mov	ax, count
	mov	word ptr xmmParms.copyLength[_off], ax
	mov	word ptr xmmParms.copyLength[_seg], 0

	lea	si, xmmParms
	mov	ah, XMM_MEMCPY
	call	xmmEntry
	.if	!ax
		; format error message
		push	ax
		mov	di, xmmParms.dstHandle
		mov	dx, xmmParms.srcHandle
		mov	cx, word ptr xmmParms.copyLength
		xor	bh, bh
		.if	dir == PAGE2MEM
			invoke	Panic, E_XMM_READ, bx, cx, dx, di
		.else
			invoke	Panic, E_XMM_WRITE, bx, cx, dx, di
		.endif
		pop	ax
	.endif

	ret

xmmMemCpy	endp

;---------------------------------------------------------------------------
; page2XMSOfs	converts a page number to a linear offset into XMS memory
;
; entry:	bx		= page number
;			dx		= offset into page
; exit:	dx:ax	= offset into XMS allocation
;---------------------------------------------------------------------------
page2XMSOfs	proc near private	uses cx

	mov	cx, dx
	mov	ax, bx
	shr	ax, 1						; convert from offset to index
	mov	dx, PAGESIZE
	mul	dx
	add	ax, cx
	adc	dx, 0						; dx:ax = offset into XMS allocation
	ret

page2XMSOfs	endp

	end
