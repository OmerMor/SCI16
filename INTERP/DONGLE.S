;-------------------------------------------------------------------
; DONGLE.S
; Dongle detection code for beta test version
;-------------------------------------------------------------------

include	macro.i
include	dongle.i

SELVAL	equ	0D7h

ifdef BETA
testact	proto near
delayact proto near
endif

.DATA

dongleCount		word	300		;we will fail after 300 cycles in dialog
noDongle			word	0

.CODE

CheckDongle	proc
ifdef BETA
	;Code to test for Dongle device
	call  testact
	.if	!ax
		;User doesn't have a dongle.  Set him up for failure.
		mov	ax, dongleCount
		mov   noDongle, ax
	.endif
endif
	ret
CheckDongle	endp

ifdef BETA

delayact proc near
;  DELAYACT Subroutine
;  ===================
;  Delays for a short amount of time specified in AX.  1 iteration is a
;  minimum of 1us on a 33Mhz '386.
;
;  In:  ax = time to delay ( 0-65535 )
;
;  Out: Delay taken
;       All registers but flags preserved

	push	cx
	mov	cx,ax
	.repeat
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
	.untilcxz
	pop	cx
	ret

delayact endp

testact	proc near
;  The TestAct routine will test for the presence of a Sierra On-Line
;  activator connected to printer port ZERO only!
;
;  Note: Interrupts turned off for duration of test (several milliseconds)
;
;  In:  No parameters
;
;  Out: Returns 1 if activator found
;	Returns 0 if no activator, wrong activator or no parallel card found
;	Interrupts restored to previous state

	pushf
	push	es
	cli
;
; Lookup printer address, make sure its valid
; -------------------------------------------
	sub	ax,ax
	mov	es,ax
	mov	bx,1032
	mov	ax,es:[bx]
	or		ax,ax
	jne	ta0
	jmp	short failact
;
; If printer address same as next 2, its net stuff (invalid)
; ----------------------------------------------------------
ta0:
	cmp	ax,[bx+2]
	jne	ta1
	cmp	ax,[bx+4]
	je		failact
;
; Read and store current info in printer port (bx)
; ------------------------------------------------
ta1:
	mov	dx,ax
	inc	dx
	inc	dx
	in		al,dx
	or		al,4
	mov	bl,al
;
; Get next piece of info to save from port, put on stack
; ------------------------------------------------------
	dec	dx
	dec	dx
	in		al,dx
	mov	bh,al
	push	bx			; Info stored on STACK !
;
; Send power-up sequence to activator
; -----------------------------------
	mov	al,128
	out	dx,al
	mov	ax,5000			; Min 5 ms.
	call	delayact
	mov	al,192
	out	dx,al
;
; Now set T-selector mode to OR
; -----------------------------
	mov	al,0CFh
	out	dx,al
	mov	al,0C0h
	out	dx,al
;
; Init test flag and counter
; --------------------------
	sub	bx,bx			; Test flag
	mov	cx,9			; Count
;
; Loop here.  Send out selector, zero, and ack enable
; ---------------------------------------------------
	.repeat
		mov	al,SELVAL
		out	dx,al
		mov	al,0C0h
		out	dx,al
		mov	al,0E0h
		out	dx,al
;
; Do short delay, test for ack, store result
; ------------------------------------------
		mov	ax,20
		call	delayact
		inc	dx
		in		al,dx
		dec	dx
		and	al,64
		or		bl,al
;
; Shut off ack enable, continue loop
; ----------------------------------
		mov	al,0C0h
		out	dx,al
	.untilcxz
;
; Now lets do an ack test, store result
; -------------------------------------
	mov	al,0E0h		; Turn on ack enable
	out	dx,al
	mov	ax,20		; Do short delay
	call	delayact
	inc	dx
	in		al,dx
	dec	dx
	and	al,64
	mov	bh,al		; bh = test result
	mov	al,0C0h		; Turn off ack enable
	out	dx,al
;
; Turn off activator, restore databyte
; ------------------------------------
	mov	al,0
	out	dx,al
	pop	cx			; Info restored from STACK  !
	mov	al,ch
	out	dx,al
;
; Now restore control byte
; ------------------------
	inc	dx
	inc	dx
	mov	al,cl
	out	dx,al
	dec	dx
	dec	dx
;
; If no ack occured, test failed
; ------------------------------
	or		bh,bh
	jne	failact
;
; If test flag was never high, fail
; ---------------------------------
	or		bl,bl
	je		failact
	mov	ax,1
	jmp	short exitact
;
; Failed entry here.  Set return value to zero
; ============================================
failact:
	sub	ax,ax
;
; Restore registers and return
; ----------------------------
exitact:
	pop	es
	popf			; May reenable interrupts
	ret

testact	endp

endif

	end

