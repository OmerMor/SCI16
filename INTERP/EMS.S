;---------------------------------------------------------------------------
; ems.s		Mark Wilden, December 1990
;
; routines for accessing expanded memory from the alternate memory manager
; see Ray Duncan, Extending DOS, for EMS details
;---------------------------------------------------------------------------

include	macro.i

include	armasm.i
include	ems.i

; DOS functions
DOS_OPEN						equ 3Dh
DOS_OPEN_READ				equ 00h

DOS_CLOSE					equ 3Eh

DOS_IOCTL					equ 44h
DOS_IOCTL_DEVICEINFO		equ 00h
DOS_IOCTL_CHARDEV			equ 80h
DOS_IOCTL_OUTPUTSTATUS	equ 07h

; EMM functions
EMM_INT				equ 67h

EMM_GETSTATUS		equ 40h
EMM_GETPAGEFRAME	equ 41h
EMM_GETNPAGES		equ 42h
EMM_ALLOC			equ 43h
EMM_MAPPAGE			equ 44h
EMM_RELEASE			equ 45h
EMM_GETVERSION		equ 46h

.data

ems ARMType <	\
	emsName,		\
	'-',			\
	init,			\
	memcpy,		\
	term,			\
	PAGESIZE		\
>

; EMM data
emmName		byte 'EMMXXXX0',0	; name of EMM device
handle		word NO_MEMORY		; handle of EMS memory we allocated
pageFrame	word ?				; segment where EMS memory is bank-switched to
emsName		byte "EMS", 0

.code

;---------------------------------------------------------------------------
; EMSDetect	determine if EMS memory is available, and if so, how much.
;
; entry:	nothing
; exit:	ax	= number of pages available
;			pageFrame global variable set
;---------------------------------------------------------------------------
EMSDetect proc uses bx dx

; open the EMM "file" to see if EMS is installed and available
	lea	dx, emmName
	dos	DOS_OPEN, DOS_OPEN_READ
	jc		noEMS

; make sure that it's not a real file, but a device
	mov	bx, ax										; bx = "file" handle
	dos	DOS_IOCTL, DOS_IOCTL_DEVICEINFO		; dx = device info
	jc		noEMS

; make sure that the device is a character device...
	and	dx, DOS_IOCTL_CHARDEV
	jz		noEMS

; ...and that it's available
	dos	DOS_IOCTL, DOS_IOCTL_OUTPUTSTATUS	; al = output status
	jc		noEMS
	or		al, al
	jz		noEMS

; close it to release the handle
	dos	DOS_CLOSE
	jc		noEMS

; there is an EMM:  see if it's active
	mov	ah, EMM_GETSTATUS
	int	EMM_INT
	or		ah, ah
	jnz	noEMS

; see what version it is (we won't touch < 3.2--if such a thing exists)
	mov	ah, EMM_GETVERSION
	int	EMM_INT
	or		ah, ah
	jnz	noEMS
	cmp	al, 32h
	jb		noEMS

; find what our page frame is
	mov	ah, EMM_GETPAGEFRAME
	int	EMM_INT
	or		ah, ah
	jnz	noEMS
	mov	pageFrame, bx

; see how many pages are available
	mov	ah, EMM_GETNPAGES
	int	EMM_INT					; bx = # available 16K pages
	or		ah, ah
	jnz	noEMS

	mov	ax, bx
	jmp	@F

noEMS:
	xor	ax, ax
@@:
	ret

EMSDetect endp

;---------------------------------------------------------------------------
; EMSAlloc	allocate EMS memory
;
; entry:	nPages	= number of 16K pages
; exit:	ax			= handle, or NO_MEMORY if error
;---------------------------------------------------------------------------
EMSAlloc proc	uses bx dx, nPages:word

	mov	ah, EMM_ALLOC
   mov	bx, nPages
	int	EMM_INT
	.if ah
		mov	ax, NO_MEMORY
	.else
		mov	ax, dx
	.endif
	
	ret

EMSAlloc endp

;---------------------------------------------------------------------------
; EMSMapPage	maps one page into EMS page frame
;
; entry:	ax		= physical page
;			bx		= logical page
;			dx		= handle
; exit:	ax		= 0 if success (ah = error code)
;---------------------------------------------------------------------------
EMSMapPage	proc uses bx dx, physicalPage:byte, logicalPage:word, aHandle:word

	mov	al, physicalPage
	mov	bx, logicalPage
	mov	dx, aHandle
	mov	ah, EMM_MAPPAGE
	int	EMM_INT
	xor	al, al

	ret

EMSMapPage endp

;---------------------------------------------------------------------------
; EMSGetPageFrame		returns page frame being used
;
; entry:	nothing
; exit:	ax	= offset of page frame
;			dx = segment of page frame
;---------------------------------------------------------------------------
EMSGetPageFrame proc

		xor	ax, ax
		mov	dx, pageFrame
		
		ret

EMSGetPageFrame endp

;---------------------------------------------------------------------------
; EMSFree	release EMS memory
;
; entry:	handle
; exit:	nothing
;---------------------------------------------------------------------------
EMSFree proc	uses ax dx, aHandle:word

		mov	ah, EMM_RELEASE
		mov	dx, aHandle
		int	EMM_INT
		
		ret

EMSFree endp

;---------------------------------------------------------------------------
; init	determine if EMS memory is available, and if so, how much.
;			allocate it all
;
; entry:	nothing
; exit:	ax	= success
;---------------------------------------------------------------------------
init	proc private

	invoke	EMSDetect
	.if !ax
		jmp noEMS
	.endif

	.if ax > MAXPAGES
		mov ax, MAXPAGES
	.endif
	mov	ems.nFreePages, ax
	mov	ems.nPages, ax
	cmp	ax, MINPAGES
	jb		noEMS

	invoke	ARMAllocPageMap, addr ems

	invoke	EMSAlloc, ems.nPages
	.if ax == NO_MEMORY
		jmp noEMS
	.endif
	
	mov	handle, ax
	jmp	initEnd

noEMS:
	mov	ems.nFreePages, 0
	mov	ems.nPages, 0

initEnd:
	mov	ax, ems.nFreePages
	mov	ems.active, ax
	ret

init	endp

;---------------------------------------------------------------------------
; memcpy		copies data to/from EMS
;
; entry:	es:di	= data
;			ax		= direction (PAGE2MEM or MEM2PAGE)
;			bx		= page
;			cx		= count (in bytes)
;			dx		= offset
; exit:	ah		= 0 if success
;---------------------------------------------------------------------------
memcpy proc private uses bx cx si di ds es

	; map the logical page into the pageframe
	mov	si, ax
	shr	bx, 1						; convert from byte offset to index
	invoke EMSMapPage, 0, bx, handle
	.if ah
		jmp @F
	.endif

	; set up registers for copy
	mov	bx, pageFrame
	.if si == MEM2PAGE
		push	es
		pop	ds
		mov	si, di      		; ds:si = data
		mov	es, bx
		mov	di, dx				; es:di = pageFrame
	.else
		mov	ds, bx
		mov	si, dx
	.endif

	; copy data
	cld
	rep movsb

	xor	ah, ah

@@:
	ret

memcpy	endp

;---------------------------------------------------------------------------
; term	release all EMS memory, if any
;
; entry:	nothing
; exit:	nothing
;---------------------------------------------------------------------------
term	proc private

	.if handle != NO_MEMORY
		invoke	EMSFree, handle
	.endif
	invoke	ARMFreePageMap, addr ems
	mov		ems.active, FALSE
	ret

term	endp
	end
