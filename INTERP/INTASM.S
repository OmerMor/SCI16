;-------------------------------------------------------------------
; INTASM.S
; Grab/restore various interrupts.
;-------------------------------------------------------------------

include	macro.i

include	audio.i
include	dialog.i
include	errmsg.i
include	fileio.i
include	intrpt.i
include	start.i

;-------------------------------------------------------------------

dosSetVector	equ	25h
dosGetVector	equ	35h

prtScrInt		equ	5
timeInt			equ	8
videoInt  		equ	10h

ctrlBrkInt	equ	1bh

charInt		equ	1fh
breakExit	equ	23h
critErrInt	equ	24h

timerPort	equ	40h	;address of counter port for interval timer
EOI			equ	20h	;end-of-interrupt to 8259

;This is the counter to load the programmable interval timer with in order
;to get interrupts at a rate of 60/second.  Clock to the PIT is 1.19318 MHz.

TIMER_DIVISOR	equ	19886
DISNEY_DIVISOR	equ	1326
DISNEY_INTS		equ	15

;Issue BIOS interrupt every 3 interrupts.

TIMER_COUNT	equ	3

;Every 11 BIOS interrupts, skip one in order to account for differences
;between our 20/sec rate and native 18.2/sec.

ADJ_COUNT  	equ	11

;-------------------------------------------------------------------
	.DATA?

timerCounter	word		?	;count left to next user interrupt
adjustCounter	word		?	;count left to adjustment time
interruptsSet	word		?	;flag to indicate that interrupts have been set
oldTimer   	   dword		?
oldCtrlBrk 	   dword		?
oldCritErr 	   dword		?
oldPrtScr  	   dword		?
sysTicks  	   dword		?
servers			Server MaxServers DUP  (<>)

;-------------------------------------------------------------------
	.DATA

diskIOCritical	word	TRUE
interruptLevel	word	0
disneyCntr		word	0

;-------------------------------------------------------------------
	.CODE

SetInterrupts		proc
;Set the address of our various interrupt handlers.

	;First, save the address of the current handlers
	push	es

	dos	dosGetVector, prtScrInt
	mov	WORD PTR oldPrtScr[_off], bx
	mov	WORD PTR oldPrtScr[_seg], es

	dos	dosGetVector, timeInt
	mov	WORD PTR oldTimer[_off], bx
	mov	WORD PTR oldTimer[_seg], es

	dos	dosGetVector, ctrlBrkInt
	mov	WORD PTR oldCtrlBrk[_off], bx
	mov	WORD PTR oldCtrlBrk[_seg], es

	dos	dosGetVector, critErrInt
	mov	WORD PTR oldCritErr[_off], bx
	mov	WORD PTR oldCritErr[_seg], es

	pop	es

	;RPoint the interrupts to our routines
	;
	;Get the segment and offset addresses for the routines to set up
	; ds <- segment
	; dx <- offset

	push	ds

	mov	ax, cs
	mov	ds, ax

	mov	dx, offset @code:NullInterrupt
	dos	dosSetVector, prtScrInt
	dos	dosSetVector, ctrlBrkInt

	mov	dx, offset @code:CritErrorHandler
	dos	dosSetVector, critErrInt

	mov	dx, offset @code:BreakExitHandler
	dos	dosSetVector, breakExit

	pop	ds

	;Do the timer interrupt set-up
	call	SetTimerInt

	mov	interruptsSet, 1

	return

SetInterrupts		endp

ResetInterrupts	proc	
;Set all interrupts back to their original handlers

	.if	interruptsSet
		push	ds
		pop	es
		lds	dx, oldPrtScr
		dos	dosSetVector, prtScrInt
		mov	ax, es
		mov	ds, ax

		lds	dx, oldCtrlBrk
		dos	dosSetVector, ctrlBrkInt
		mov	ax, es
		mov	ds, ax

		call	ResetTimerInt

		lds	dx, oldCritErr
		dos	dosSetVector, critErrInt
		mov	ax, es
		mov	ds, ax
	.endif

	return

ResetInterrupts	endp

SetTimerInt	proc	near

	;Initialize the user timer interrupt counter.
	mov	timerCounter, TIMER_COUNT
	mov	adjustCounter, ADJ_COUNT

	;Point the interrupt vector to our code.
	push	ds
	mov	dx, offset @code:TimerIntHandler
	mov	ax, cs
	mov	ds, ax
	dos	dosSetVector, timeInt
	pop	ds

	;Change the timer interrupt frequency to 60/second
	mov	al, 36h			;timer 0, LSB, MSB, mode 3
	out	timerPort+3, al
	mov	ax, TIMER_DIVISOR
	out	timerPort, al
	xchg	ah, al
	out	timerPort, al
	return

SetTimerInt	endp

SetDisneyInt	proc	
	cli
	mov	al, 36h			;timer 0, LSB, MSB, mode 3
	out	timerPort+3, al
	mov	ax, DISNEY_DIVISOR
	out	timerPort, al
	xchg	ah, al
	out	timerPort, al
	mov	disneyCntr, DISNEY_INTS
	sti
	return

SetDisneyInt	endp

ResetTimerInt	proc	near   
;Set the timer frequency back to normal (18.2/second)

	mov	al, 36h			;timer 0, LSB, MSB, mode 3
	out	timerPort+3, al
	xor	al, al
	out	timerPort, al
	out	timerPort, al

	;Restore the old interrupt vector.
	lds	dx, oldTimer
	dos	dosSetVector, timeInt
	mov	ax, es
	mov	ds, ax

	return

ResetTimerInt	endp

TimerIntHandler		proc	far 
; This routine must be far.
; This routines gets the interrupt from the timer chip and passes 
; it down the server list.  Every 20th of a second, passes it to 
; the old BIOS low-level timer interrupt, to keep the system moving 
; as expected.

	pushall
	push	ds
	push	es

	mov	bx, @data
	mov	ds, bx
	mov	es, bx
	;Make sure that string operations take place in the 
	;expected direction.
	cld

	; If the Disney Sound Source is in use, the interrupt rate is 900 per second
	; So we will call SCI's standard 60 per sec interrupt every 15th time through
	cmp	disneyCntr, 0
	.if	disneyCntr
		dec	disneyCntr
		.if	!zero?
			mov	al, EOI
			out	20h, al
			mov	bx, audioDrv
			.if	bx
				mov	ax, A_STAT
				call	dword ptr [bx]
			.endif

			pop	es
			pop	ds
			popall
			iret
		.endif
		mov	disneyCntr, DISNEY_INTS
	.endif

	 ;Update the current time.
	 inc	WORD PTR sysTicks
	 .if	zero?
	   inc	WORD PTR sysTicks[2]
	 .endif

	 ;Finish up the interrupt by either issuing the EOI or 
	 ;passing it on to the machine's interrupt handler, which 
	 ;will finish it for us.
	 dec	byte ptr timerCounter
	 .if	!zero?
DoEOI:
	    ;Nothing to do -- issue the end-of-interrupt
	    mov	al, EOI
	    out	20h, al
	 .else
	    dec	adjustCounter
	    .if	zero?
	 	   ;Time to adjust for the difference between our
	 	   ;20/second interrupts and the native 18.2/second
	 	   ;by not issuing this interrupt.
	 	   mov	timerCounter, TIMER_COUNT
	 	   mov	adjustCounter, ADJ_COUNT
	 	   jmp	DoEOI
	    .endif

	    ;Time to call the BIOS routine (which will issue EOI for us)
	    mov	timerCounter, TIMER_COUNT
	    pushf
	    call	oldTimer
	 .endif

	;Call each server in the timer interrupt list.
   lea   bx, servers
assume	bx:ptr Server
   mov   cx, MaxServers
	.repeat
		.if	[bx].inUse
		   push  bx									;save current server and count
		   push  cx

			;Decrement this server's counter.  If it reaches zero, 
			;reload it and call the server.
			dec	[bx].count
			.if	zero?
				mov	ax, [bx].freq
				mov	[bx].count, ax
				call	[bx].function
			.endif
		   pop   cx									;Restore pointer to server and count
			pop	bx
		.endif
		add	bx, type Server
	.untilcxz
assume	bx:nothing

	pop	es
	pop	ds
	popall
	iret

TimerIntHandler	endp

NullInterrupt	proc	far 
;This routine must be far.

	iret

NullInterrupt	endp

BreakExitHandler	proc	far
;Deal with abnormal termination due to a critical DOS error.
;This routine must be far.

	invoke	exit, 1

BreakExitHandler	endp

CritErrorHandler	proc	far
;Handle DOS critical errors by putting up an alert and either retrying or
;aborting the opertation.
;This routine must be far.

	;Allow interrupts so that we can get the user's response.
	sti

	;Save those registers which DOS wants saved.
	push	bx
	push	cx
	push	dx
	push	ds
	push	es

	;Get back data addressability.
	mov	ax, @data
	mov	ds, ax
	mov	es, ax

	cmp	diskIOCritical, 5586					; inhibit critical error message?
	.if	zero?
		xor	ax, ax								; inhibit critical error message!
	.else
		;Create an error message and print the alert.
		and	di, 0ffh								;mask off undefined portion of error code

		.if	diskIOCritical == TRUE
			mov	dx, E_QUIT
		.else
			mov	dx, E_CANCEL
		.endif

		sub	sp, 200

		invoke	ReadErrMsg, dx, sp			; On return ax points to msg
		mov	dx, ax

		mov	ax, E_DISK_ERROR_MSG				; Find correct error message between
		add	ax, di								; messages 4 - 16

		sub	sp, 200
		mov	bx, sp

		push	dx
		invoke	ReadErrMsg, ax, bx			; On return ax points to msg
		pop	dx										; Restore previous msg

		invoke	RAlert, E_DISK_ERROR, ax, dx

		add	sp, 400			; return stack pointer
	.endif

	;Test the return value to see what to do.
	or		ax, ax

	;Restore the registers which DOS wants
	pop	es
	pop	ds
	pop	dx
	pop	cx
	pop	bx

	;Test the return value to see what to do.
	.if	!zero?
		;Have DOS try again.
		mov	al, 1
	.else
		;We'll be handling this one way or another -- 
		;get back addressability.
		mov	ax, @data
		mov	ds, ax
		mov	es, ax

		.if	diskIOCritical == TRUE
			;We can't run without this disk IO - 
			;terminate the program.
			invoke	exit, 1
		.else
			;We can run without the IO -- return an error.

			add	sp,6								;Pop DOS's return address off the stack

			;Set the carry flag in the flags which will be returned to the caller
			mov	bp,sp
			mov	ax,[bp+22]						;get the original flags
			or		ax,1								;set the carry
			mov	[bp+22],ax						;and put the flags back

			;Let everybody know that this is a critical 
			;error which we're returning from.
			mov	criticalError, 1

			;Restore all registers from the stack
			pop	ax
			pop	bx
			pop	cx
			pop	dx
			pop	si
			pop	di
			pop	bp
			pop	ds
			pop	es
		.endif
	.endif

	iret

CritErrorHandler	endp

_cli	proc

	cli
	return

_cli	endp

_sti	proc

	sti
	return

_sti	endp

SysTime	proc	func:word
;Return word of system time data

	.if	func == 3
		; return packed calendar date
		; from DOS cx = 1980-2099Y dh=1-12M dl=1-31D
		; return form YYYY|YYYM|MMMD|DDDD
		; Get Date from DOS
		mov	ah, 2ah
		int	21h
		mov	ah, dh
		xor	al, al
		shr	ax, 1
		shr	ax, 1
		shr	ax, 1
		or		al, dl
		; subtract the base of our year 
		; numbering system
		sub	cx, 1980
		shl	cx, 1
		or		ah, cl
	.else
		; return packed time
		; from DOS ch=0-23H cl=0-59M dh=0-59S
		; return form either:
		; HHHH|MMMM|MMSS|SSSS
		; OR
		; HHHH|HMMM|MMMS|SSSS
		; * note loss of SECOND resolution in this form

		mov	ah, 2ch
		int	21h									; Get Time from DOS

		mov	ah, cl								; free up cl and start building return
		xor	al, al								; minutes bits span ah, al
		shr	ax, 1
		shr	ax, 1
		or		al, dh								; put seconds in zeroed bottom 6 bits

		;hours go in top bits of ah after we massage them
		.if	func == 2
			; 24 hour request
			shr	ax, 1								; make room for 5 bits of hours
			mov	cl, 3
		.else
			; must be adjusted into proper range (ie. 0:30 -> 12:30)
			.if	!ch
				add	ch, 12
			.endif
			.if	ch > 12
				sub	ch, 12
			.endif
			mov	cl, 4
		.endif
		shl	ch, cl
		or		ah, ch
	.endif

	ret

SysTime	endp

	end

