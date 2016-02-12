;-------------------------------------------------------------------
; MOUSEASM.S
; Low level mouse interface for Microsoft compatible mice.
;
; NOTE:
;
; This entire module can be converted to C as soon as we start using
; a library that allows access to software interrupts, assuming the
; compiler allows C interrupt functions
;-------------------------------------------------------------------

include macro.i

include event.i
include eventasm.i
include config.i
include debug.i
include mouse.i
include resource.i
include restypes.i
include video.i

MAXY				equ 200
MAXX				equ 320

CURSORVIEW  	equ 998

mouseInt			equ 33h
mouseInstalled	equ 0
mouseShow		equ 1
mouseHide		equ 2
mouseGet			equ 3
mouseSet			equ 4
mouseCursor		equ 9
mouseSetInt		equ 12

mouseMoved		equ 01h
lButDown			equ 02h
lButUp			equ 04h
rButDown			equ 08h
rButUp			equ 10h
cButDown			equ 20h
cButUp			equ 40h

butDown			equ lButDown or rButDown or cButDown
butUp				equ lButUp or rButUp or cButUp

lButton			equ lButDown or lButUp
rButton			equ rButDown or rButUp
cButton			equ cButDown or cButUp

;-------------------------------------------------------------------
	.DATA

anEvent		Event	<>								;the local event record
haveMouse	word 	0								;do we have a mouse?
mouseY		word 	0
mouseX		word 	0
patchX		word 	0
mPoint		RPoint<>

;-------------------------------------------------------------------

	.CODE

InstallMouse	proc
;See if we're supposed to use a mouse.
;If so, check to see if one is present.

	mov	ax, useMouse
	.if	ax
		; try for a MS MOUSE
		mov	ax, mouseInstalled
		int	mouseInt
	.endif
	mov	haveMouse, ax

	.if	ax
		;Read the mouse's initial position
		;and scale if > 300
		mov	ax, mouseGet
		int	mouseInt
		.if	cx >= 300
			mov	patchX, 0ffffh
			shr	cx, 1
		.endif
		mov	mouseX, cx
		mov	mouseY, dx

		; do the first show
	  ; set mouse cursor to default cursor
	  invoke    ResCheck, RES_VIEW, CURSORVIEW
	  cmp   ax, 0
	  je    showCursor

	  invoke    ResLoad, RES_VIEW, CURSORVIEW
	  mov   si, ax
	  invoke    RSetCursor, si, 0, 0, -1, -1

showCursor:
		call	RShowCursor
		invoke	MoveCursor, mouseX, mouseY

		; set user interrupt subroutine
		push	es
		mov	ax, cs
		mov	es, ax
		mov	cx, 7fh								;all mouse events
		mov	dx, offset @code: HndlMouse
		mov	ax, mouseSetInt
		int	mouseInt
		pop	es
  .else
		call	RShowCursor
	.endif

	ret

InstallMouse	endp

DisposeMouse	proc	uses si di
;Unhook timer tick routine and shut down the mouse

	.if	haveMouse
		mov	ax, mouseInstalled
		int	mouseInt
	.endif
	ret

DisposeMouse	endp

SetMouse	proc	uses si, mpoint:ptr Point
;Put local point in mouse coords

	mov	si, mpoint
	mov	bx, rThePort
assume	si:ptr RPoint
assume	bx:ptr RGrafPort
	mov	dx, [si].v
	add	dx, [bx].origin.v
	mov	cx, [si].h
	add	cx, [bx].origin.h
assume	si:nothing
assume	bx:nothing

	.if  sword ptr	cx < 0
		mov	cx, 0
	.endif
	.if sword ptr	cx >= MAXX
		mov	cx, MAXX-1
	.endif
	mov	mouseX, cx

	.if sword ptr	dx < 0
		mov	dx, 0
	.endif
	.if  sword ptr	dx >= MAXY
		mov	dx, MAXY-1
	.endif
	cmp	dx, MAXY
	mov	mouseY, dx

	.if	haveMouse
		.if	patchX & 1
			shl	cx, 1
		.endif

		; call the mouse
		mov	ax, mouseSet
		int	mouseInt
	.endif

	mov	ax, mouseX
	mov	bx, mouseY
	invoke	MoveCursor, ax, bx

	ret

SetMouse	endp

;-----------------------------------------------------------------------
; Far event proc handler for Mouse generated interrupt
; This must be a far proc
;-----------------------------------------------------------------------

HndlMouse	proc	far
;Mouse interupt handler. build an event record and post it

	pushall
	push	ds

	cli
	cld

	mov	si, @data
	mov	ds, si

	.if	isDebugVersion && mouseIsDebug && (ax & cButDown)
		call	ToggleDebug
	.else
		;Set current mouse information
		mov	buttonState, bx
		.if	patchX & 1
			shr	cx, 1
		.endif
		.if	cx >= MAXX
			mov	cx, MAXX-1
		.endif
		mov	mouseX, cx
		mov	mouseY, dx
		mov	anEvent.where.h, cx
		mov	anEvent.where.v, dx
		
		;Only create events for non-move incidents
		.if	ax != mouseMoved
			push	ax										;save the interrupting condition

			;See whether a button went up or down.
			.if	ax & butDown
				mov	cx, mouseDown
			.else
				mov	cx, mouseUp
			.endif
			mov	anEvent.eType, cx

			;Center button events are treated as ctrl-left button events.
			;Set the modifiers accordingly.
			.if	ax & cButton
				mov	bx, ctrl
			.else
				xor	bx, bx
			.endif

			;Right button events are treated as shift-left button events.
			;Set the modifiers accordingly.
			.if	ax & rButton
				mov	bx, shft
			.endif
			mov	anEvent.modifiers, bx

			call	GetModifiers
			or		anEvent.modifiers, ax

			;Post the event.
			invoke	RPostEvent, addr anEvent
			pop	ax
		.endif

	;If the mouse moved, we must move cursor to a new location.
	.if	ax & mouseMoved

			mov	cx, mouseX
			mov	dx, mouseY
				

; If we are restricted to a rectangle act accordingly
		.if	restrictRecPtr != 0
			assume si: ptr RRect
			mov	si, restrictRecPtr
			

			.if	cx < 	[si].left
				mov	cx,[si].left
			.endif
			.if	cx > [si].right
				mov	cx, [si].right
 			.endif
 			.if	dx < [si].top
 				mov	dx, [si].top
 			.endif
			.if	dx > [si].bottom
 				mov	dx, [si].bottom
 			.endif	  
			assume si: nothing


			mov	mouseX, cx
			mov	mouseY, dx


			assume	si:ptr RPoint
			assume	di:ptr RGrafPort

			mov	di,  rThePort
			mov	si, OFFSET mPoint
			sub	cx, [di].origin.h
			mov	[si].h, cx
			sub	dx, [di].origin.v
			mov	[si].v, dx 
			invoke	SetMouse ,si

			assume	si:nothing
			assume	di:nothing

		.endif

			invoke	MoveCursor, mouseX, mouseY
		.endif
	.endif

	pop	ds
	popall
	ret

HndlMouse	endp

	end

