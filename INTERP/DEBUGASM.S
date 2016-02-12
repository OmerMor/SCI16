;-------------------------------------------------------------------
; DEBUGASM.S
;
; Low level code to handle debugging.
;-------------------------------------------------------------------

include macro.i

include debug.i
include debugasm.i
include list.i
include memmgr.i
include object.i
include pmachine.i
include resource.i
include selector.i
include start.i
include stdio.i
include string.i

;-------------------------------------------------------------------

ifndef	DEBUG
NoDebugError	proto		reason:word
else
PrintName		proto near
endif

;-------------------------------------------------------------------

;Free list structure for pointers
block	struc
	bSize		word	?	;size of pointer
	bNext		word	?	;pointer to next free block in list
block	ends

;-------------------------------------------------------------------
	.DATA

ifdef	DEBUG

debuggerIsOn		word	FALSE
opTableSaveHandle	word	0
freeMsg				byte	"%x@%x  ", 0	; DEBUGGING

endif

;-------------------------------------------------------------------

	.CODE		PM_TEXT

DebugInit	proc

ifdef	DEBUG
	invoke	GetResHandle, 512					;256 * sizeof(memptr)
	mov	opTableSaveHandle, ax
endif

	return

DebugInit	endp

RDebugger	proc	near
;This code is called in lieu of the op-code when debugging is on.  We
;save the relevant registers and call the debugger, then do the dispatch
;to the op-code.

ifndef	DEBUG

   invoke	NoDebugError, 11

else

	push	ax
	push	cx
	mov	cx, es
	push	es
	mov	di, opTableSaveHandle				;get handle to opCode storage
	mov	ax, [di+_seg]							;get segment of opCode storage
	mov	es, ax
	push	es:[bx]									;save address of op-code

	;Call the debugger
	invoke	Debug, si, cx, bp
	
	;Restore everything and dispatch to the op-code.
	pop	bx											;recover address of op-code
	pop	es
	mov	di, scriptHandle
assume	di:ptr hunkPtr
  	mov   cx, [di].hSegment
assume	di:nothing
	.if	cx
	  	mov   	es, cx
		.endif
	pop	cx
	pop	ax
	push	pmCodeSegment
	push	bx

endif

	retf

RDebugger	endp

KStackUsage	proc	uses si di bp, funcPtr:word

ifndef	DEBUG
   invoke	NoDebugError, 12
else
	mov	si, funcPtr
	invoke	StackUsage, [si+2]			;pass function
	mov	acc, ax
endif
	ret

KStackUsage	endp

;-----------------------------------------------------------------------
;         From here on only compiled if debugging enabled
;-----------------------------------------------------------------------
ifdef	DEBUG

DebugOn	proc	uses si di

;Turn debugging on.  This is done by replacing the entries in the op-code
;dispatch table in pmachine.s with the address of _Debugger.  The real
;addresses for the op-codes are squirreled away in storage allocated in
;hunk space.

	;Don't turn debugger on if it already is on...
	mov	ax, debuggerIsOn
	.if	!ax
		mov	debuggerIsOn, TRUE

		;Point the es at the allocated hunk and copy the current dispatch
		;table to it.
		mov	si, opTableSaveHandle
		mov	es, [si+_seg]
		xor	di, di
		mov	si, offset @data:opTbl
		mov	cx, 256
		rep	movsw

		;Now fill the dispatch table with the address of our debugger
		;dispatch routine, _Debugger.
		mov	ax, ds
		mov	es, ax
		mov	ax, offset RDebugger
		mov	di, offset @data:opTbl
		mov	cx, 256
		rep	stosw
	.endif

	ret

DebugOn	endp

DebugOff	proc	uses si di

;Turn the debugger off by replacing the dispatch table for the op-codes
;with their true values.

	; don't turn ourself off if we aren't on
	mov	ax, debuggerIsOn
	.if	ax
		mov	debuggerIsOn, FALSE
		
		;Copy the saved op-code addresses to opTbl.
		mov	si, opTableSaveHandle
		push	ds
		mov	ax, ds
		mov	es, ax
		mov	ax, [si+_seg]
		mov	ds, ax
		xor	si, si
		mov	di, offset @data:opTbl
		mov	cx, 256
		rep	movsw
		pop	ds
	.endif

	ret

DebugOff	endp

StackUsage    	proc	uses si di bp, func:word

;Return information on stack usage, depending on the value of 'function':
;	0	size of processor stack
;	1	maximum processor stack used
;	2	current usage of processor stack
;	3	size of PMachine stack
;	4	maximum PMachine stack used
;	5	current usage of PMachine stack

	mov	ax, func

	.if	!ax
		;Return processor stack size.
		mov	ax, STACKSIZE
		jmp	SUExit
	.endif

	dec	ax
	.if	zero?
		;Return max processor stack used.
		mov	di, stackBase
		mov	al, 's'
		mov	cx, STACKSIZE
		repe	scasb
		mov	ax, cx
		jmp	SUExit
	.endif

	dec	ax
	.if	zero?
		;Return current processor stack used (approx.).
		mov	ax, 0fffeh
		sub	ax, sp
		jmp	SUExit
	.endif  

	dec	ax
	.if	zero?
		;Return PMachine stack size.
		mov	ax, PSTACKSIZE
		jmp	SUExit
	.endif

	dec	ax
	.if	zero?
		;Return max PMachine stack used.
		mov	di, pStack
		mov	cx, PSTACKSIZE
		add	di, cx
		dec	di
		mov	al, 'S'
		std
		repe	scasb
		cld
		mov	ax, cx
		jmp	short SUExit
	.endif

	;Return current PMachine stack usage.
	mov	ax, pmsp
	sub	ax, pStack

SUExit:
	ret

StackUsage		endp

ifdef	PROFILER

ProfileHandler	proc   uses es di si

	mov	di, proBuffer
	.if	(proOn == TRUE) && di
		mov	ax, [di+2]			;get segment of profile buffer
		mov	es, ax
		mov	di, proIndex		;get index into buffer
		mov	si, ssPtr

     mov   bx, [si]
assume	bx:ptr Obj
	   mov	ax, [bx].oInfo					;get the -info- property
	   and	ax, CLONEBIT or NODISPOSE	;is this really a clone?
	   .if	ax == CLONEBIT
		   ;Object is a disposable clone, so save the
        ;permanent object that was cloned
        mov   ax, [bx].oSuper
        stosw                         ; store object's super
        add   si, 2                   ; point to selector
        movsw
     .else
		   movsw							      ;put object in buffer
  		movsw							      ;put selector in buffer
     .endif

		.if	di >= proBufSize
			xor	di, di
		.endif
		mov	proIndex, di
	.endif

	return

ProfileHandler	endp

DoTrace		proc	uses es di
;ax = selector
;bx = object

	mov	di, proBuffer
	.if	di
		;Get pointer to current buffer position
		mov	di, [di+2]			;get segment of profile buffer
		mov	es, di
		mov	di, proIndex		;get index into buffer

		;Compute depth of stack
		push	ax

		mov	ax, ssPtr
		sub	ax, offset @data:sendStack
		mov	es:[di], ax

assume	bx:ptr Obj
	   mov	ax, [bx].oInfo					;get the -info- property
	   and	ax, CLONEBIT or NODISPOSE	;is this really a clone?
	   .if	ax == CLONEBIT
		   ;Object is a disposable clone, so save the
        ;permanent object that was cloned
        mov   ax, [bx].oSuper
        mov   es:[di+2], ax           ; store object's super
     .else
  		mov	es:[di+2], bx           ;Put object in buffer
     .endif

     ;Put selector in buffer
     pop   ax
  	mov	es:[di+4], ax

		;Update buffer pointer
		add	di, 6
		.if	di >= proBufSize
			xor	di, di
		.endif
		mov	proIndex, di
	.endif

	return

DoTrace		endp

endif

ShowFreeList	proc	uses si di, where:word
;Display the free list.

	mov	si, freeList
	.while	si
		mov	di, where
		invoke	strlen, di
		add	di, ax
assume	si:ptr block
		invoke	sprintf, di, addr freeMsg, [si].bSize, si
		mov	si, [si].bNext
assume	si:nothing
	.endw

	ret

ShowFreeList	endp

;-------------------------------------------------------------------------------
;Display hunk space.
;-------------------------------------------------------------------------------
if 0
ShowHunks	proc	uses si di

	invoke	printf, addr hunkHdr, hunkList

	xor	si, si			;offset into hunk
	mov	di, hunkBase	;segment address of hunk
	.while	di < hunkTop
		mov	es, di
		mov	ax, es:[si].hSize
		test	word ptr es:[si].status, ALLOCATED
		.if	zero?
			mov	bx, offset @data:freeStr
		.else
			test	word ptr es:[si].status, LOCKED
			.if	zero?
				mov	bx, offset @data:allocStr
			.else
				mov	bx, offset @data:lockStr
			.endif
		.endif
		invoke	printf, addr hunkMsg, di, ax, bx

		mov	es, di
		test	word ptr es:[si].status, ALLOCATED
		.if	zero?
			mov	ax, es:[si].hNext
			invoke	printf, addr nextMsg, ax
		.else
			invoke	putchar, LF
		.endif

		mov	es, di
		add	di, es:[si].hSize
	repeat

	ret
ShowHunks	endp
endif

;-------------------------------------------------------------------------------
;Show the list of active handles and the hunks they reference.
;-------------------------------------------------------------------------------
if 0
ShowHandles	proc	uses si

	invoke	printf, addr handleHdr, handleBase, nextHandle, lowHandle

	mov	si, handleBase
	.while	si > lowHandle
		;Print handle information.
		mov	ax, [si+_seg]
		or		ax, [si+_off]
		.if	zero
			invoke	printf, addr freeHndlMsg, si
		.else
			mov	ax, [si+_seg]
			invoke	printf, addr handleMsg, si, ax
		.endif

		sub	si, HANDLESIZE
	repeat

	ret
ShowHandles	endp
endif

endif

	end
