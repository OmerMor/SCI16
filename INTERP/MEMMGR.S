;--------------------------------------------------------------------------
;MEMMGR.S
;
;Routines to handle memory management for both the heap and hunk space.
;There are two types of memory which can be allocated:
;
;Pointers:
;	These will generally be used to build lists, etc. and will
;	prefereably be implemented with a processor's smallest
;	pointer size for speed.
;
;Hunks:
;	These will be used for space for data loaded from disk, 
;	background save areas, etc.  These should be implemented
;	in such a way as to give the largest address space possible.
;
;Revision History:
;
;90.03.23 Abdul
;	Changed to use .MODEL directive and simplified segments.  
;	Microsoft MASM 5.1.
;
;90.03.16 Larry Scott & Corinna Abdul
;	Added check for debugging (-d option used when invoking the 
;	script interpreter) to determine if should go into the sci
;	debugger on the error "Out of heap space.".
;
;88.04.01 8:21 JAS
;	Changed to use a fixed-size handle table.
;
;87.08.16 16:4	JAS
;	CopyHandle added. InitMem now allocates as much memory as it can,
;	but insists on at least hunkSize paragraphs.  Fixed FillPtr
;
;87.05.06 15:51	JAS
;	Module created.
;--------------------------------------------------------------------------


;--------------------------------------------------------------------------

include macro.i
include debug.i
include dialog.i
include errmsg.i
include memmgr.i
include pmachine.i
include resource.i
include start.i
include stdio.i
include stdlib.i
include string.i

;-------------------------------------------------------------------------------

;Status bits for hunkHeaders
LOCKED			equ	1
ALLOCATED		equ	2
CRITICAL			equ	80h

;These are the minimum sizes for free block or hunks.
MIN_BLOCK_SIZE	equ	4		;in bytes
MIN_HUNK_SIZE	equ	2		;in paragraphs (16 bytes)

HANDLESIZE		equ	4
LONGSIZE		   equ	4
WORDSIZE		   equ	2

AllocMem		   equ	48h

;-------------------------------------------------------------------------------
;			DATA AND INITIALIZATION
;-------------------------------------------------------------------------------


;Free list structure for hunks
hunk	struc
	hNext		word	?	;pointer to next free hunk in list
	hSize		word	?	;size of hunk
	status	word	?	;status of this hunk
hunk	ends

;-------------------------------------------------------------------------------

	.DATA?

hunkAvail		word	0ffffh
heapAvail		word	?
heapBase			word	?
handleBase		word	?
handlePtr		word	?
restartBase		word	?

;All hunk addresses are paragraph addresses (real addresses/16)
hunkBase 		word	?	;address of base of hunk space
hunkTop			word	?	;top of hunk space
hunkList 		word	?	;segment of starting hunk in free hunk list

handle			word	?
prevFree 		word	?
packDone 		word	?

savedHeapStart	word	?


;-------------------------------------------------------------------------------


	.DATA

;hunkHdr		byte	"Hunks:", LF, "    hunkList -> %x", LF, 0
;nextMsg		byte	": next -> %x", LF, 0
;hunkMsg		byte	"    %x -- %x paragraphs: %s", 0
;freeStr		byte	"free", 0
;allocStr		byte	"allocated", 0
;lockStr		byte	"locked", 0

;hunkErrMsg	byte	"Check Hunk Error no:%d",0



;-------------------------------------------------------------------------------

	.CODE


;-------------------------------------------------------------------------------
;Initialize the heap from which pointers will be allocated, and the
;handle table.
;-------------------------------------------------------------------------------
InitHeap	proc	uses di, theBase:word, theSize:word

	;Zero out the heap.
	xor	ax, ax
	mov	di, theBase
	mov	cx, theSize
	mov   heapAvail, cx
	shr	cx, 1
	rep	stosw
	
	;Initialize the handle table, which is at the top of the heap.
	mov	ax, theBase
	add	ax, theSize
	mov   bx, numHandles
	shl   bx, 1										;each handle takes 4 bytes
	shl   bx, 1
ifdef DEBUG
	;Following code sets up a duplicate handle table
	; if X command line option has been specified
	.if	checkingLoadLinks
		shl   bx, 1
	.endif
endif
	sub   ax, bx
	mov	handleBase, ax
	mov	handlePtr, ax

	;Set up the initial free block.
	mov	bx, theBase
	mov	heapBase, bx
	sub	ax, bx									;ax = _handleBase
assume	bx:ptr block
	mov	[bx].bSize, ax
	mov	[bx].bNext, 0
assume	bx:nothing

	;RPoint the free list to this block.
	mov	freeList, bx
	ret

InitHeap	endp


;-------------------------------------------------------------------------------
;These routines manage the hunks -- the large pieces of memory accessed by
;a far pointer.
;-------------------------------------------------------------------------------
InitHandle	proc	uses si, baseSeg:word, paraSize:word

	;Set up the header for hunk memory.
	mov	ax, baseSeg
	mov	es, ax
	xor	si, si
assume	si:ptr hunk
	mov	es:[si].hNext, si  					;no next block
	mov	es:[si].status, si					;unlocked and free
	mov	bx, paraSize
	mov	es:[si].hSize, bx
assume	si:nothing

	mov	hunkList, ax
	mov	hunkBase, ax
	add	ax, bx
	mov	hunkTop, ax
	ret

InitHandle	endp


;-------------------------------------------------------------------------------
; initialize memory
;-------------------------------------------------------------------------------
InitMem	proc	uses di, maxHunkSize:word

	;Zero out the BSS data.
	cld

	mov	di, offset @data:bssStart			;beginning of bss area
	mov	cx, offset @data:bssEnd				;end of bss area
	sub	cx, di
	xor	ax, ax
	rep	stosb										;zero bss

	;Set up the free list with the memory lying between static
	;data and the stack.  Make sure that the heap starts on a
	;word boundary.
	mov	ax, offset @data:bssEnd				;get start of free mem
	.if	ax & 1
		inc	ax
	.endif
	mov	bx, stackBase		
	sub	bx, ax
	invoke	InitHeap, ax, bx

	;Get hunk space and initialize it and the handles.
	mov	bx, maxHunkSize
	dos	AllocMem

	; if we didn't get what we wanted, we get what we can get
	.if	carry?
		mov	maxHunkSize, bx
		dos	AllocMem
	.endif
	mov	hunkAvail, bx
	
	; we get to use BX paragraphs starting at para AX
	invoke	InitHandle, ax, bx

	ret

InitMem	endp



;-------------------------------------------------------------------------------
;				POINTERS
;-------------------------------------------------------------------------------
;
;On the IBM & compatibles, 'freeList' points to the first block in a
;free list.  This block has a header which contains a pointer to the
;next block in the list or NULL if it is at the end of the list.
;A first-fit strategy is used to allocate the block.  When allocating
;a block, enough extra space is allocated that the allocated block size
;can be stored at the beginning of the block -- the pointer returned
;is to the first byte past this size.  This size is used when we free
;the block.
;-------------------------------------------------------------------------------

RNewPtr		proc	uses si di, theSize:word
;-------------------------------------------------------------------------------
;Return a pointer to 'size' bytes in the heap, or NULL if there is no
;free block large enough to do so.  Always allocate in multiples of words.
;
;Register usage:
;	bx	pointer to previous block in list
;	si	pointer to current block in list
;	di	pointer to next block in list
;	cx	required block size
;-------------------------------------------------------------------------------

	;The required size is the requested size plus enough space to
	;hold the allocated block size.  Round up to an even number of bytes.
	mov	cx, theSize
	.if	!cx
		invoke Panic, E_HEAP_ALLOC
	.endif

	add	cx, WORDSIZE
	.if	cx & 1
		inc	cx
	.endif

assume	si:ptr block
assume	di:ptr block
assume	bx:ptr block

	;Search the free list for a free block large enough.
	xor	bx, bx							;previous ptr = NULL
	mov	si, freeList					;pointer to start of list
	.while	si && cx > [si].bSize
		mov	bx, si
		mov	si, [si].bNext
	.endw

	;If the current block pointer is NULL, there was no block large enough.
	.if	!si
		xor	ax, ax
		return
	.endif

	;If the block we've found isn't big enough to break in two, 
	;allocate the whole thing.  Otherwise, break it up.
	mov	ax, [si].bSize
	sub	ax, cx
	.if	ax < MIN_BLOCK_SIZE
		mov	cx, [si].bSize				;set required size = block size
		mov	di, [si].bNext				;get pointer to next block
	.else
		;Create a new block following this one.
		mov	di, si
		add	di, cx
		mov	ax, [si].bNext
		mov	[di].bNext, ax				;point new block to next block in list
		mov	ax, [si].bSize
		sub	ax, cx
		mov	[di].bSize, ax				;set new block's size
	.endif
	
	;Link around the allocated block.
	.if	bx
		mov	[bx].bNext, di				;Link the previous block to the next block.
	.else
		mov	freeList, di				;Current block was first block in the list --
	.endif									; point the list head to the next block

	;Put the block size in the first word of the current block.
	mov	[si].bSize, cx

	;Return the pointer to the free space in the block.
	add	si, WORDSIZE
	mov	ax, si

assume	si:nothing
assume	di:nothing
assume	bx:nothing

	return


RNewPtr		endp



NeedPtr		proc	theSize:word
;-------------------------------------------------------------------------------
;Try to get a pointer and abort the program if we can't.
;-------------------------------------------------------------------------------

	invoke	RNewPtr, theSize
	.if	!ax
		invoke RAlert, E_NO_HEAP
ifdef DEBUG
		invoke	SetDebug, TRUE
		invoke	Debug, ip, pmsp
endif
		invoke	exit, 1
	.endif
	return

NeedPtr	endp



FillPtr	proc	uses di, thePtr:word, theByte:byte
;-------------------------------------------------------------------------------
;Fill the block of memory pointed to by thePtr with theByte.
;-------------------------------------------------------------------------------

	mov	ax, ds
	mov	es, ax
	mov	al, theByte
	mov	di, thePtr	      		;get pointer to memory
	mov	cx, [di-WORDSIZE]     		;get size of block
	sub	cx, WORDSIZE	      		;minus size of header
	rep	stosb
	ret

FillPtr	endp



ClearPtr	proc	thePtr:word
;-------------------------------------------------------------------------------
;Clear the block of memory pointed to by thePtr.
;-------------------------------------------------------------------------------

	invoke	FillPtr, thePtr, 0
	ret

ClearPtr	endp



DisposePtr	proc	uses si di, thePtr:word
;-------------------------------------------------------------------------------
;Return a block to the free list, coalescing it with neighboring free
;blocks if necessary.
;
;Register usage
;	bx	pointer to previous block in list
;	si	pointer to returned block
;	di	pointer to next block in list
;-------------------------------------------------------------------------------

	;Convert the returned block to a memory block.
	mov	si, thePtr
	sub	si, WORDSIZE			 ;point back to block start

assume	si:ptr block
assume	di:ptr block
assume	bx:ptr block

	;Search from the base of the list until we hit a block whose address
	;is higher than that of the returned block.
	xor	bx, bx				 					;previous ptr = NULL
	mov	di, freeList			 				;pointer to head of list
	.while	di && di <= si
		mov	bx, di
		mov	di, [di].bNext
	.endw

	;Find the end of the block being returned.
	mov	ax, si
	add	ax, [si].bSize

	;Compare to start of next block.
	.if	ax != di
		;Can't coalesce -- just link into chain.
		mov	[si].bNext, di
	.else
		;They match -- merge the blocks.
		mov	ax, [di].bNext
		mov	[si].bNext, ax
		mov	ax, [di].bSize
		add	[si].bSize, ax
	.endif

	;If there is a previous block, see if we can coalesce with it.
	.if	!bx
		;No previous block -- the returned block is at
		;the start of the free list.
		mov	freeList, si
	.else
		;There is a previous block -- see if we can coalesce with it.
	 	;We can coalesce if the end of the previous block is the byte
	 	;preceeding the returned block.
		mov	ax, bx
		add	ax, [bx].bSize			;ax = pointer to end of previous block
		.if	ax != si
			;Can't coalesce -- just link into chain.
			mov	[bx].bNext, si
		.else
			;The previous and returned blocks abut -- merge them.
			mov	ax, [si].bNext
			mov	[bx].bNext, ax
			mov	ax, [si].bSize
			add	[bx].bSize, ax
		.endif
	.endif

assume	si:nothing
assume	di:nothing
assume	bx:nothing

	ret

DisposePtr	endp



ReallocatePtr	proc	uses si di, thePtr:word, newSize:word
;-------------------------------------------------------------------------------
;Reallocate a pointer to a different size, but at the same address.
;Return pointer if successful, NULL otherwise.
;
;Register usage:
;	si	pointer being reallocated
;	di	portion of block being returned
;	dx	requested size
;-------------------------------------------------------------------------------

	mov	si, thePtr								;get pointer
	sub	si, WORDSIZE							;point to block
	mov	di, si
	mov	cx, [si]									;get current size of block
	mov	dx, newSize								;and requested size
	add	dx, 2										;add 2 for header
	add	di, dx									;pointer to portion being returned

	;Current implementation only allows for shrinking a block.  Check
	;for other cases and return NULL.
	xor	ax, ax									;prepare a null return
	.if	cx >= dx
		sub	cx, dx								;compute amount to return
		.if	cx >= MIN_BLOCK_SIZE
			;Create a block consisting of the of the unneeded portion of the
			;block being reallocated, and return it.
			mov	[di], cx
			add	di, 2					;point past size of block
			invoke	DisposePtr, di

			;Set the size of the reallocated block.
			mov	[si], dx
		.endif
		mov	ax, si						;return pointer
	.endif

	ret

ReallocatePtr	endp



LargestPtr	proc	uses si
;-------------------------------------------------------------------------------
;Return the size of the largest block we can get with the RNewPtr() call.
;
;Register usage:
;	si	pointer to memory block in free list
;	ax	size of largest memory block
;-------------------------------------------------------------------------------

	xor	ax, ax
	mov	si, freeList
assume	si:ptr block
	.while si
		;Check the size of this block.
		mov	bx, [si].bSize
		.if	bx > ax
			mov	ax, bx
		.endif
		mov	si, [si].bNext
	.endw
assume	si:nothing

	;Subtract the header size which will be required.
	.if	ax
		sub	ax, WORDSIZE
	.endif
	ret

LargestPtr	endp



FreeHeap	proc	uses si di
;-------------------------------------------------------------------------------
;Return the total free heap space.  Zero free space in the process.
;-------------------------------------------------------------------------------

	mov	ax, ds
	mov	es, ax

	xor	ax, ax
	mov	bx, ax
	mov	si, freeList
assume	si:ptr block
	.while	si
		;Add the size of this block.
		mov	cx, [si].bSize		;get size (will be used below)
		add	bx, cx

		;Zero the block.
		mov	di, si
		add	di, type block
		sub	cx, type block
		rep	stosb								;ax is always 0

		;Move down the list.
		mov	si, [si].bNext
	.endw
assume	si:nothing

	mov	ax, bx
	ret

FreeHeap	endp



PtrSize	proc	thePtr:word
;-------------------------------------------------------------------------------
;Return the size of the block corresponding to a pointer.
;-------------------------------------------------------------------------------

	mov	bx, thePtr	 							;get pointer to memory
	mov	ax, [bx-WORDSIZE]						;get size of block
	sub	ax, WORDSIZE	 						;minus size of header
	ret

PtrSize	endp



SaveHeap	proc

	mov	ax, freeList
	mov	restartBase, ax
	return

SaveHeap	endp



RestartHeap	proc
;-------------------------------------------------------------------------------
;Restore the heap to its initial pristine state of one large free block.
;-------------------------------------------------------------------------------

	;Restore the beginning of the free list
	mov	bx, restartBase
	mov	freeList, bx
assume	bx:ptr block

	;Compute the length of this block from the bottom of the
	;handle table and create a node here.
	mov	dx, handleBase
	sub	dx, bx
	mov	word ptr [bx].bNext, 0
	mov	[bx].bSize, dx
assume	bx:nothing

	return

RestartHeap	endp



memcpy	proc	uses si di, dest:near ptr byte, source:near ptr byte, count:word
;-------------------------------------------------------------------------------
;Copy memory from one place to another.
;-------------------------------------------------------------------------------

	mov	ax, ds
	mov	es, ax
	mov	si, source
	mov	di, dest
	mov	cx, count
	.if	si < di
		;Start copy from end of blocks to ensure that dest doesn't
		;trash source.
		add	si, cx
		dec	si
		add	di, cx
		dec	di
		std
	.endif
	rep	movsb
	cld
	ret

memcpy	endp



;-------------------------------------------------------------------------------
;				HANDLES
;-------------------------------------------------------------------------------
;These routines do management of the handles (i.e. the pointers to hunk
;pointers kept at the top of heap space).
;-------------------------------------------------------------------------------
;-------------------------------------------------------------------------------
;				HUNKS
;-------------------------------------------------------------------------------
;These routines manage the hunks -- the large pieces of memory accessed by
;a far pointer.
;-------------------------------------------------------------------------------


RelocateHunk	proc uses si di
;-------------------------------------------------------------------------------
;dx points to an unlocked, allocated hunk, bx to a free hunk at
;a lower address.  Relocate the allocated hunk to the space occupied
;by the free hunk, adjust its handle to reflect this, create a free hunk
;following the relocated one, and return the pointer to this hunk in dx.
;-------------------------------------------------------------------------------

assume	si:ptr hunk

	;If this block is 'critical' (i.e. is being accessed by code running
	;on interrupts), turn interrupts off during the relocation.
	.if	es:[si].status & CRITICAL
		cli
	.endif

	;Find the handle to the allocated hunk and adjust to its new address.
	mov	ax, dx
	inc	ax											;point to data area of hunk
	mov	di, handleBase
	mov   cx, numHandles
   shl   cx, 1										;4 bytes per handle
	shl   cx, 1
	add   cx, di
	.while	ax != [di+_seg]
		add	di, HANDLESIZE
		.if	di == cx
			;There is an allocated hunk with no handle!
ifdef DEBUG
			.if	checkingLoadLinks
				mov   checkingLoadLinks, FALSE
			.endif
endif
			mov	bx, ax
			invoke	PError, word ptr thisIP, pmsp, E_PACKHANDLE_FAILURE, bx
		.endif
	.endw
	mov	[di+_seg], bx
ifdef DEBUG
	;If X option keep a parallel handle table.
	.if	checkingLoadLinks
		push  di
		add   di,numHandles     ;4 bytes per handle
		add   di,numHandles
		add   di,numHandles
		add   di,numHandles
		mov	[di+_seg], bx
		inc	word ptr [di+_seg]
		pop   di
	.endif
endif
	inc	word ptr [di+_seg]					;point to data area of hunk
	;Now move the hunk down to the free space.
assume	di:ptr hunk
	xor	si, si
	xor	di, di
	mov	es, bx									;point to free hunk
	push	es:[di].hNext							;save pointer to next free hunk
	push	es:[di].hSize							;and hunk's size
	push	ds											;save our data segment
	mov	ds, dx									;point to allocated hunk
	mov	cx, [si].hSize							;get size of allocated hunk
	shl	cx, 1
	shl	cx, 1
	shl	cx, 1										;cx = size of hunk in words
	rep	movsw										;move the allocated hunk down
	pop	ds

	;Move has been done -- reenable interrupts if they've been disabled.
	xor	si, si
	.if	es:[si].status & CRITICAL
		sti
	.endif

	;The hunk has been moved.  Now build a free hunk above
	;the relocated allocated hunk.
	add	bx, es:[si].hSize						;bx = seg addr of new hunk
	mov	es, bx
	pop	es:[si].hSize
	pop	es:[si].hNext
	mov	es:[si].status, si					;not allocated or locked

	;Tell the previous hunk about this new one.
	mov	cx, prevFree
	.if	!cx
		;Then this is the first hunk in the free list.
		mov	hunkList, bx
	.else
		;Set the next pointer in the previous hunk to point
		;to this hunk.
		mov	es, cx
		mov	es:[si].hNext, bx
		mov	es, bx
	.endif

	;Check to see if we can coalesce with the next free block
	;(which, after all, is the point of this whole exercise).
	mov	dx, bx									;return pointer to this block
	add	bx, es:[si].hSize
	.if	bx < hunkTop && bx == es:[si].hNext
		;Then we can coalesce.
		push	es
		mov	es, bx
		mov	ax, es:[si].hSize					;get size of next block
		mov	cx, es:[si].hNext					;and pointer to next
		pop	es
		add	es:[si].hSize, ax
		mov	es:[si].hNext, cx
	.endif

assume	si:nothing
assume	di:nothing
	return

RelocateHunk	endp



PackHandles	proc	uses si di
;-------------------------------------------------------------------------------
;Pack hunk space in hopes of opening up larger free hunks.
;
;Register usage:
;	bx	segment of the current free area
;	dx	segment of hunk which we're looking at
;-------------------------------------------------------------------------------
ifdef DEBUG
	.if	checkingLoadLinks
		;Check for internal consistancy of parallel handle tables
		push  cx
		mov	di, handleBase
		mov   si, di
		mov   cx, numHandles
		shl   cx, 1
		shl   cx, 1
		add   si, cx
		cld
		push  es
		push  ds
		pop   es
		repe  cmpsb
		pop   es
		pop   cx
		.if	!zero?
			mov   checkingLoadLinks, FALSE
			dec   di
			invoke	PError, word ptr thisIP, pmsp, E_PACKHANDLE_HEAP, di
		.endif
		call	CheckLoadLinks
		.if	!ax
			mov	checkingLoadLinks, FALSE
			dec	di
			invoke	PError, word ptr thisIP, pmsp, E_PACKHANDLE_HUNK, di
		.endif
	.endif
endif

	mov	prevFree, 0		;initialize last free hunk pointer
	mov	dx, hunkBase
	.while TRUE
		xor	si, si		;zero offset into hunk
assume	si:ptr hunk

		;Find a free hunk.
		.while dx < hunkTop
			;Test for a free hunk.
			mov	es, dx
			.break .if !(es:[si].status & ALLOCATED)

			;Not free -- point to next hunk.
			add	dx, es:[si].hSize
		.endw
		mov	bx, dx		;bx = segment of free hunk
		.break .if dx >= hunkTop

		;Point to the next hunk and check to see if we're done.
		add	dx, es:[si].hSize
		.break .if dx == hunkTop

		;The next hunk must be allocated (since we coalesce returned
		;hunks).  If it is locked, skip around it and begin looking
		;for free blocks above it.
		mov	es, dx
		.if	es:[si].status & LOCKED
			mov	prevFree, bx
			add	dx, es:[si].hSize				;point to next hunk
		.else
			call	RelocateHunk					;relocate the hunk
		.endif
	.endw

assume	si:nothing
	ret

PackHandles	endp


GetHandle	proc	uses si di	cx
;-------------------------------------------------------------------------------
;Return a new handle.
;
;Register usage:
;	si	pointer to handle table
;
;	Must preserve bx, cx, dx.
;-------------------------------------------------------------------------------

	;Scan for a new handle.
	mov	si, handlePtr
	mov	cx, si

	mov   di, numHandles
	;4 bytes per handle
	shl   di, 1
	shl   di, 1
	add   di, handleBase

	.repeat
		;Move on to the next handle
		add	si, HANDLESIZE
		.if	si >= di
			mov	si, handleBase
		.endif

		;Test for a free handle
		mov	ax, [si+_seg]
		or		ax, [si+_off]
	.until zero? || si == cx

	.if	ax
		xor	ax, ax
	.else
		mov	ax, si
		mov	handlePtr, ax
	.endif

	ret

GetHandle	endp



RNewHandle	proc	uses si di, lsize:word
;-------------------------------------------------------------------------------
;Return a pointer to a handle.  Handles are pointers to pointers to a hunk.
;
;Register usage:
;	bx	segment of previous hunk
;	cx	segment of current hunk
;	dx	segment of next hunk
;	di	requested size (in paragraphs)
;	si	always 0 -- offset of base of hunk
;-------------------------------------------------------------------------------

	mov	packDone, 0								;no pack of handles has been done
TryHandle:
	;Get the requested size, convert it to the appropriate number
	;of paragraphs (16 byte blocks), and add an extra paragraph for
	;the header.
	mov	di, lsize								;get requested size
	.if	!di
		invoke Panic, E_HUNK_ALLOC
	.endif

	add	di, 15									;round up to paragraph
	mov	cl, 4
	shr	di, cl									;convert to paragraph
	inc	di											;add a paragraph for the header

	;Search the hunk list for a free hunk large enough.
	xor	bx, bx									;previous hunk = NULL
	mov	si, bx
assume	si:ptr hunk
assume	di:ptr hunk
	mov	cx, hunkList
	.while	cx
		;Point to the current hunk.
		mov	es, cx

		;Is this hunk large enough for the allocation?
		.break .if di <= es:[si].hSize

		;Block not large enough -- move on to the next one.
		mov	bx, cx								;previous = current
		mov	cx, es:[si].hNext
	.endw

	.if	cx
		;Got a large enough hunk, get a handle to put it in.
		call	GetHandle
	.else
		;We're at the end of the list, so we couldn't find a hunk big enough.
		;If we've already tried packing the handles, we're sunk.  Otherwise, 
		;pack the handles and try allocating again.
		.if	packDone
			;A pack has been done -- no hope for us now
			xor	ax, ax
		.else
			inc	packDone
			call	PackHandles
			jmp	TryHandle
		.endif
	.endif
	or		ax, ax
	jz		NewHandleExit

	mov	handle, ax

	;Compute the size of the hunk which would be left free if the
	;requested allocation were split out of the hunk
	mov	ax, es:[si].hSize
	sub	ax, di

	;If the remaining free hunk wouldn't be big enough to be useful, 
	;just allocate the whole hunk.  Otherwise, split the hunk.
	.if	ax < MIN_HUNK_SIZE
		;Then the hunk is small -- allocate the whole thing.
		mov	di, es:[si].hSize
		mov	dx, es:[si].hNext					;pointer to next hunk
	.else
		mov	dx, cx
		add	dx, di								;pointer to new hunk
		push	es:[si].hNext						;save pointer to next hunk
		mov	es, dx								;point to new hunk
		mov	es:[si].hSize, ax					;set its size (which was in ax)
		pop	es:[si].hNext						;set pointer to next
 		mov	es:[si].status, si				;not allocated
	.endif

	;Link around the allocated hunk.
	.if	bx
		mov	es, bx								;point to previous hunk
		mov	es:[si].hNext, dx					;and set its next pointer
	.else
		;Then we're at the head of the hunk list -- point to
		;the next hunk.
		mov	hunkList, dx
	.endif
	
	;Put the new size in the structure and mark it as allocated and unlocked.
	mov	es, cx									;point back to allocated hunk
	mov	es:[si].hSize, di
	mov	word ptr es:[si].status, ALLOCATED
assume	si:nothing
assume	di:nothing

	;Point to the data area of the hunk.
	inc	cx

	;Put the pointer to the hunk in the handle and return the handle.
	mov	ax, handle
	mov	si, ax
	mov	[si+_seg], cx
	mov	word ptr [si+_off], 0
ifdef DEBUG
	; if X option keep a parallel handle table.
	.if	checkingLoadLinks
		push  si
		add   si,numHandles     ;4 bytes per handle
		add   si,numHandles
		add   si,numHandles
		add   si,numHandles
		mov	[si+_seg], cx
		mov	word ptr [si+_off], 0
		pop   si
	.endif
endif

NewHandleExit:
	ret

RNewHandle	endp



NeedHandle	proc	lsize:word
;-------------------------------------------------------------------------------
;Try to get a handle and abort the program if we can't.
;-------------------------------------------------------------------------------

	invoke	RNewHandle,lsize	   			;try getting the handle
	.if	!ax
		invoke Panic, E_NO_HUNK_SPACE
	.endif
	return

NeedHandle	endp



FillHandle	proc	uses di, theHandle:word, theByte:word
;-------------------------------------------------------------------------------
;These routines manage the hunks -- the large pieces of memory accessed by
;a far pointer.
;-------------------------------------------------------------------------------
;Fill the hunk of memory pointed to by 'theHandle' with 'theByte'.
;-------------------------------------------------------------------------------

	mov	di, theHandle
	mov	ax, [di+_seg]							;get segment of hunk
	dec	ax											;point to hunk header
	mov	es, ax
	xor	di, di
	mov	bx, es:(hunk ptr [di]).hSize						;get size of hunk (paragraphs)
	dec	bx											;minus header size
	mov	cl, 3
	shl	bx, cl
	mov	cx, bx									;cx = hunk size (words)
	inc	ax											;point back to hunk data
	mov	es, ax
	mov	al, byte ptr theByte
	mov	ah, al
	rep	stosw
	ret

FillHandle	endp



ClearHandle	proc	theHandle:word
;-------------------------------------------------------------------------------
;Clear the hunk of memory pointed to by 'theHandle'.
;-------------------------------------------------------------------------------

	invoke	FillHandle, theHandle, 0
	ret

ClearHandle	endp



ReturnHandle	proc	uses si, theHandle:word
;-------------------------------------------------------------------------------
;Return 'theHandle' as a free handle.
;
;Register usage:
;	si	returned handle
;-------------------------------------------------------------------------------

	;Zero out the handle.
	mov	si, theHandle
	xor	ax, ax
	mov	[si+_seg], ax
	mov	[si+_off], ax
ifdef DEBUG
	;Following code clears the handle in a duplicate handle table
	; if X command line option has been specified
	.if	checkingLoadLinks
		push  si
		add   si, numHandles
		add   si, numHandles
		add   si, numHandles
		add   si, numHandles
		mov	[si+_seg], ax
		mov	[si+_off], ax
		pop   si
	.endif
endif
	ret

ReturnHandle	endp



DisposeHandle	proc	uses si di, theHandle:word
;-------------------------------------------------------------------------------
;These routines manage the hunks -- the large pieces of memory accessed by
;a far pointer.
;-------------------------------------------------------------------------------
;Return the hunk pointed to by a handle to the free list.
;-------------------------------------------------------------------------------

	xor	si, si

	;Get the pointer to the hunk data from the handle, then decrement
	;it to get a pointer to the hunk header.
	mov	di, theHandle
	mov	cx, [di+_seg]							;get segment of returned hunk
	dec	cx											;point to hunk header
	
	;Dispose of the hunk.
	call	DisposeHunk
	
	;Return the handle.
	invoke	ReturnHandle, theHandle

	ret

DisposeHandle	endp



DisposeHunk	proc near
;-------------------------------------------------------------------------------
;Return a hunk to the hunk list, coalescing it with neighboring free
;hunks if necessary.
;
;Register usage:
;	bx	segment of previous hunk
;	cx	segment of hunk being returned
;	dx	segment of next hunk
;	si	always 0 -- offset of base of hunk
;-------------------------------------------------------------------------------

	;Mark the hunk as free.
	mov	es, cx
assume	si:ptr hunk
	mov	es:[si].status, si

	;Search from the base of the list until we hit a hunk whose address
	;is higher than that of the returned hunk.
	xor	bx, bx				;previous hunk = NULL
	mov	dx, hunkList
	.while dx && dx < cx
		mov	bx, dx
		mov	es, dx
		mov	dx, es:[si].hNext
	.endw
	.if	dx == cx
		invoke Panic, E_RET_HUNK
	.endif

	;Find the end of the hunk being returned
	mov	ax, cx
	mov	es, cx
	add	ax, es:[si].hSize

	;Compare to start of next hunk.
	.if	ax != dx
		;Can't coalesce -- just link into chain.
		mov	es:[si].hNext, dx
	.else
		;We can coalesce hunks.
		mov	es, dx								;point at next hunk in list
		mov	di, es:[si].hNext					;get pointer to its next hunk
		mov	ax, es:[si].hSize
		mov	es, cx								;point at returned hunk
		add	es:[si].hSize, ax
		mov	es:[si].hNext, di
	.endif

	;Check for a previous hunk in the list.
	.if	!bx
		;No previous hunk -- returned hunk is the start of the
		;hunk list.
		mov	hunkList, cx
	.else
		;There is a previous block -- see if we can coalesce with it.
		;We can coalesce if the end of the previous block is the byte
		;preceeding the returned block.
		mov	es, bx
		mov	ax, bx
		add	ax, es:[si].hSize					;ax = end of previous hunk
		.if	ax != cx
			;Can't coalesce -- just link into chain.
			mov	es, bx							;point to previous hunk
			mov	es:[si].hNext, cx
		.else
			;Then we can coalesce the hunks.
			mov	es, cx							;point to returned hunk
			mov	ax, es:[si].hNext
			mov	di, es:[si].hSize
			mov	es, bx							;point to previous hunk
			add	es:[si].hSize, di
			mov	es:[si].hNext, ax
		.endif
	.endif
assume	si:nothing

	return

DisposeHunk	endp



ReallocateHandle	proc	uses si, theHandle:word, newSize:word
;-------------------------------------------------------------------------------
;Reallocate a hunk to a different size, but at the same address.
;Return handle if successful, NULL otherwise.
;
;Register usage:
;	dx	segment of hunk being reallocated
;	cx	segment of portion of block being returned
;	si	always 0 -- offset of base of hunk
;-------------------------------------------------------------------------------

	;Get requested size and convert to paragraphs.
	mov	bx, newSize
	add	bx, 15
	mov	cl, 4
	shr	bx, cl
	inc	bx											;add one paragraph for header

	;Get pointer to the hunk.
	mov	si, theHandle							;get handle
	mov	dx, [si+_seg]							;get segment of hunk
	dec	dx											;point to hunk header
	mov	es, dx
	xor	si, si
assume	si:ptr hunk

	;Current implementation only allows for shrinking a block. Check
	;for other cases and refuse to do it.
	mov	ax, es:[si].hSize
	.if	bx >= ax
		xor	ax, ax
	.else
		sub	ax, bx								;compute amount to return
		.if	ax > MIN_HUNK_SIZE
			;Set the new size of the current block.
			mov	es:[si].hSize, bx
			
			;Create a block consisting of the of the unneeded portion of the
			;block being reallocated, and return it.
			mov	cx, dx
			add	cx, bx							;address of portion being returned
			mov	es, cx
			mov	es:[si].hSize, ax
			mov	es:[si].status, ALLOCATED
			call	DisposeHunk
		.endif
		mov	ax, theHandle
	.endif
assume	si:nothing

	ret

ReallocateHandle	endp



LargestHandle	proc
;-------------------------------------------------------------------------------
;Return the size of the largest hunk we can get with the RNewHandle() call.
;
;Register usage:
;	ax	size (in paragraphs) of largest hunk
;	cx	segment of hunk being examined
;	si	always 0 -- offset of base of hunk
;-------------------------------------------------------------------------------

	xor	ax, ax
	xor	bx, bx
assume	bx:ptr hunk
	mov	cx, hunkList
	.while cx
		;Compare hunk size with maximum.
		mov	es, cx
		.if	ax < es:[bx].hSize
			mov	ax, es:[bx].hSize
		.endif
		mov	cx, es:[bx].hNext
	.endw

	;Decrease block size by the required header size.
	.if	ax
		dec	ax
	.endif

	;Convert the paragraph size to bytes and return it.
	xor	dx, dx
	shl	ax, 1
	rcl	dx, 1
	shl	ax, 1
	rcl	dx, 1
	shl	ax, 1
	rcl	dx, 1
	shl	ax, 1
	rcl	dx, 1
assume	bx:nothing

	ret

LargestHandle	endp



FreeHunk	proc
;-------------------------------------------------------------------------------
;Return the total free hunk space (in paragraphs).
;
;Register usage:
;	ax	total size (in paragraphs) of free hunks
;	cx	segment of hunk being examined
;	bx	always 0 -- offset of base of hunk
;-------------------------------------------------------------------------------

	xor	ax, ax
	xor	bx, bx
assume	bx:ptr hunk
	mov	cx, hunkList
	.while cx
		mov	es, cx
		add	ax, es:[bx].hSize
		mov	cx, es:[bx].hNext
	.endw
assume	bx:nothing

	ret

FreeHunk	endp



HandleSize	proc	theHandle:word
;-------------------------------------------------------------------------------
;Return the size of the block pointed to by theHandle
;-------------------------------------------------------------------------------

	mov	bx, theHandle
	mov	ax, [bx+_seg]							;get segment of hunk
	dec	ax											;point to hunk header
	mov	es, ax
	xor	bx, bx
	mov	ax, es:(hunk ptr [bx]).hSize		;get size of hunk (paragraphs)
	dec	ax											;minus header size
	mov	cl, 4
	shl	ax, cl									;* 16 for byte size
	ret

HandleSize	endp



HandleLocked	proc	theHandle:word
;-------------------------------------------------------------------------------
;Return TRUE if handle locked
;-------------------------------------------------------------------------------

	mov	bx, theHandle							;get the handle
	mov	ax, [bx+_seg]							;get segment address of the hunk
 	dec	ax
	mov	es, ax
	xor	bx, bx
	mov	ax, es:(hunk ptr [bx]).status
	and	ax, LOCKED
	ret

HandleLocked	endp



LockHandle	proc	theHandle:word
;-------------------------------------------------------------------------------
;Lock the hunk pointed to by theHandle
;-------------------------------------------------------------------------------

	mov	bx, theHandle							;get the handle
	mov	ax, [bx+_seg]							;get segment address of the hunk
 	dec	ax
	mov	es, ax
	xor	bx, bx
	or		es:(hunk ptr [bx]).status, LOCKED	;lock the hunk
	ret

LockHandle	endp



UnlockHandle	proc	theHandle:word
;-------------------------------------------------------------------------------
;Unlock the hunk pointed to by theHandle
;-------------------------------------------------------------------------------

	mov	bx, theHandle							;get the handle
	mov	ax, [bx+_seg]							;get segment address of the hunk
	dec	ax
	mov	es, ax
	xor	bx, bx
	and	es:(hunk ptr [bx]).status, not LOCKED	;unlock the hunk
	ret

UnlockHandle	endp



CriticalHandle	proc	theHandle:word, critical:word
;-------------------------------------------------------------------------------
;Mark a handle as 'critical', i.e. not to be relocated with interrupts on.
;-------------------------------------------------------------------------------

	mov	bx, theHandle							;get the handle
	mov	ax, [bx+_seg]							;get segment address of the hunk
	dec	ax
	mov	es, ax
	xor	bx, bx
assume	bx:ptr hunk
	.if	critical
		or		es:[bx].status, CRITICAL
	.else
		and	es:[bx].status, not CRITICAL
	.endif
assume	bx:nothing
	ret

CriticalHandle	endp



hunkcpy	proc	uses si di ds, dest:word, src:dword, len:word
;Copy 'len' bytes of memory (in the hunk) at the far pointer srcSeg:srcOfs
;to 'dest' (in the heap).

	mov	ax, DGROUP
	mov	es, ax									;get destination segment (heap) in es
	mov	di, dest									;get offset of destination
	push	di											;save pointer to destination
	mov	cx, len									;get # of bytes to move
	lds	si, src									;get source pointer in ds:si
	rep	movsb										;do the move
	pop	ax											;recover pointer for return
	return

hunkcpy	endp



CopyHandle	proc	uses si, theHandle:word, thePtr:word
;-------------------------------------------------------------------------------
;Copy a hunk (specified by a handle) to heap space and return a pointer
;to it.  If thePtr is null, allocate space in the heap.  Otherwise,
;copy to that address.
;Register usage:
;	dx = size of hunk
;-------------------------------------------------------------------------------

	;Determine the size of the hunk we're to copy.
	mov	bx, theHandle
	mov	ax, [bx+_seg]		 					;get para address of handle
	dec	ax											;point to handle header
	mov	es, ax
	xor	si, si
assume	si:ptr hunk
	mov	dx, es:[si].hSize						;get the paragraph size
	mov	cl, 4
	shl	dx, cl									;get size in bytes

	;If thePtr is NULL, allocate the requisite space in the heap
	mov	ax, thePtr
	.if	!ax
		push	dx
		push	bx
		invoke	NeedPtr, dx
		pop	bx
		pop	dx
		.endif

	mov	bx, [bx+_seg]
	invoke	hunkcpy, ax, bx::si, dx
assume	si:nothing

	return

CopyHandle	endp



StackLeft	proc

	mov	ax, sp
	sub	ax, stackBase
	return

StackLeft	endp



;CheckHunk proc uses si di bp,	chErrorNo:word
;	xor	si, si			;previous hunk = NULL
;	mov	cx, hunkList
;	do
;		;At end of the list?
;		or		cx, cx
;		breakif	zero
;
;		;RPoint to the current hunk.
;		mov	es, cx
;
;      ;Is this segment out of range of hunkBase to hunkTop?
;      cmp   cx, hunkBase
;      jb    ErrorInHunkList
;      cmp   cx, hunkTop
;      jae   ErrorInHunkList
;
;		;RMove on to the next block.
;		mov	cx, es:[si+hNext]
;	repeat
;   jmp   short KeepGoing
;ErrorInHunkList:
;	 mov	bx, offset @data:hunkErrMsg ;get pointer to specific error
;	 mov	cx, chErrorNo		 ;pointer to error message
;	 sub	sp, 400				;create some space for a message
;	 mov	ax, sp
;	 invoke	sprintf, ax, bx, cx
;	 mov	ax, sp
;	 invoke	RAlert, ax
;	 add	sp, 400
;	
;KeepGoing:
;   mov   ax, ds
;   mov   es, ax
;	ret
;CheckHunk endp


FarMemcpy	proc	uses si di, dest:far ptr byte, source:far ptr byte, count:word
;-------------------------------------------------------------------------------
;Copy memory from one place to another with the source being far and dest near
;-------------------------------------------------------------------------------

	push	ds

	mov	si, word ptr source
	mov	ax, word ptr source + 2
	mov	ds, ax

	mov	ax, word ptr dest + 2
	mov	es, ax
	mov	di, word ptr dest
	mov	cx, count
	rep	movsb

	pop	ds
	ret

FarMemcpy	endp


	end
