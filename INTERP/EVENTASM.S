;-------------------------------------------------------------------
; EVENTASM.S - Low level system specific event routines
; Modifications:  
;   3/26/90 Abdul.  Changed to use .MODEL directive and simplified
;                   segments.  Microsoft MASM 5.1.
;-------------------------------------------------------------------


;-------------------------------------------------------------------
include	macro.i
include	event.i

include	start.i
include	mouse.i
include	debug.i
include	intrpt.i


;-------------------------------------------------------------------

	;Shift-Shift-D toggles debugger.
	debugKey			equ	'-'
	easyDebugKey	equ 	'`'

debugMod		equ	3
eventSize	equ	type Event

;-------------------------------------------------------------------

	.DATA

keyEvent	Event	<>

;-------------------------------------------------------------------

	.CODE


PollKeyboard	proc	uses si di bp
; build a keyboard event 

	mov	si, offset @data:keyEvent
	mov	keyEvent.modifiers, 0
	mov	bp, INP_POLL
	call	keyboard
	call	HandleEvent

	ret

PollKeyboard	endp



FlushKeyboard	proc	uses si di bp
; flush events from keyboard buffer

assume	si:ptr Event

	.repeat
		lea	si, keyEvent
		mov	bp, INP_POLL
		call	keyboard
	.until ![si].eType
	
	ret

assume	si:nothing

FlushKeyboard	endp



KeyboardDriver	proc	uses si di, function:word, event:word

	mov	si, event
	mov	bp, function
	call	keyboard
	
	ret
	
KeyboardDriver	endp



GetModifiers	proc	uses si di bp
;Return state of modifier keys (bottom 4 bits only )

	mov	bp, INP_MODIFIERS
	call	keyboard

	ret
	
GetModifiers	endp
	


PollJoystick	proc	uses si di bp

	;Create an event record on the stack.
	sub	sp, eventSize
	mov	si, sp

	mov	ax, word ptr sysTicks+_off
	mov	dx, word ptr sysTicks+_seg
	mov	bp, INP_POLL
	call	joystick
	cli
	call	HandleEvent
	sti

	add	sp, eventSize

	ret

PollJoystick	endp



JoystickDriver	proc	uses si di, function:word, event:word

	mov	si, event
	mov	bp, function
	.if	word ptr joystick+_seg != 0
		call	joystick
	.endif

	ret

JoystickDriver	endp



HandleEvent	proc	near
;If appropriate, post the event in anEvent.  Event type is in the AX.

assume	si:ptr Event

	.if	word ptr [si].eType == nullEvt
		return
	.endif

	;Get the modifiers for the key
	push	ax
	mov	ax, mouseX
	mov	[si].where.h, ax
	mov	ax, mouseY
	mov	[si].where.v, ax
	call	GetModifiers
	or		[si].modifiers, ax
	and	ax, debugMod
	mov	bx, ax
	pop	ax

	.if	(ax == keyDown && \
				(([si].message == debugKey && bx == debugMod) || \
				(isDebugVersion && [si].message == easyDebugKey)) \
			)
		cli
		call	ToggleDebug
		sti
		return
	.endif

	invoke	RPostEvent, si
	return

assume	si:nothing

HandleEvent	endp

	end
