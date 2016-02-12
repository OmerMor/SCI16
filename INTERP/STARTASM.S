;-------------------------------------------------------------------
; START.S
; Startup module for SCI
;-------------------------------------------------------------------

include	macro.i

include	errmsg.i
include	pmachine.i
include	sci.i
include	setjmp.i
include	start.i

;-------------------------------------------------------------------

;This is so the MSC compiler will have this symbol, which it
;generates an extern for when compiling.

public	_acrtused
_acrtused = 9876h

;-------------------------------------------------------------------

	.CONST

byte	"Script Interpreter, Copyright (C) 1987-1993 Sierra On-Line, Inc.", LF,
		"All rights reserved", LF, 0

;-------------------------------------------------------------------

	.DATA?

bssStart	label	byte

MAX_CMDLINE	equ	128
MAX_QUITSTR	equ	256

; 'quitStr' utilizes the fact that 'commandLine' follows it immediately
; and can be overwritten as soon as SCI begins interpretation...
quitStr			byte	MAX_QUITSTR-MAX_CMDLINE dup (?)
commandLine		byte	MAX_CMDLINE dup (?)
panicStr		   word	?

;-------------------------------------------------------------------
	.DATA

pspSeg			word	?
stackBase		word	?
numHandles     word	700

;-------------------------------------------------------------------
ENDDATA	SEGMENT PARA STACK 'STACK'
bssEnd label byte
ENDDATA	ENDS

;-------------------------------------------------------------------
	.STACK	80h

;-------------------------------------------------------------------
	.CODE

Start	proc

	mov	dx, @data
	mov	ds, dx
	mov	pspSeg, es

	;Shrink the process' allocated block to give us a 64K data segment.
	mov	bx, @data
	sub	bx, pspSeg								;size to base of data segment
	add	bx, HEAPSIZE
	dos	4Ah
	.if	carry?
		invoke	Panic, E_NO_HEAP_MEM
	.endif
	mov	ax, @data
	mov	es, ax									;set es to data segment

	;Put the stack at the top of the heap and fill it with 's's.
	mov	di, @data
	mov	ss, di
	mov	sp, 0fffeh
	mov	cx, STACKSIZE
	xor	di, di
	sub	di, cx
	mov	stackBase, di
	mov	al, 's'
	rep	stosb

	invoke MakeCommandLineArgs, 80h, pspSeg

	invoke main, argc, addr argv

	invoke exit, 0

Start	endp

setjmp		proc

	;get return address in DX:AX
	pop	ax
	pop	dx

	;get pointer to buffer (structure)
	pop	bx

	;fill in jmp structure
assume	bx:ptr jmp_buf
	mov	[bx].jmp_bp, bp
	mov	[bx].jmp_sp, sp
	mov	[bx].jmp_si, si
	mov	[bx].jmp_di, di
	mov	[bx].jmp_es, es
	mov	[bx].jmp_seg, dx
	mov	[bx].jmp_off, ax
assume	bx:nothing

	; replace argument and return address
	push	bx
	push	dx
	push	ax

	; return 0
	xor	ax, ax
	return

setjmp		endp

longjmp	proc

	; toss seg:off of return address of caller
	pop	ax
	pop	ax

	pop	bx											; get pointer to buffer
	pop	ax											; get value to return to set_jmp's caller

	; restore registers from buffer
assume	bx:ptr jmp_buf;
	mov	bp, [bx].jmp_bp
	mov	sp, [bx].jmp_sp
	mov	si, [bx].jmp_si
	mov	di, [bx].jmp_di
	mov	es, [bx].jmp_es

	; put buffer back on stack
	push	bx

	; push return address from buffer
	push	[bx].jmp_seg
	push	[bx].jmp_off
assume	bx:nothing

	; return to caller with the current value in AX
	return

longjmp	endp

;--------------------------------------------------------------------------
; WriteChar		write a text char to the screen (replace in SCI32)
;
; entry:	chr	= char to display
; exit:	nothing;
;--------------------------------------------------------------------------
WriteChar	proc	chr: byte

	mov	bx, 7				; page 0, color 7
	mov	al, chr
	bios	10h, 14
	ret

WriteChar	endp

;---------------------------------------------------------------------------
; ExitFromC		terminate program (replace with intdos() call in SCI32)
;
; entry:	code	= program exit code
; exit:	nothing
;---------------------------------------------------------------------------
ExitFromC	proc	exitCode: byte

	mov	al, exitCode
	dos	4Ch
	ret

ExitFromC	endp

	end	Start