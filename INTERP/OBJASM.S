;-------------------------------------------------------------------
; OBJECT.S
; PMachine routines for object-related operations.
;-------------------------------------------------------------------

include macro.i

include debug.i
include debugasm.i
include errmsg.i
include memmgr.i
include pmachine.i
include script.i
include selector.i
include string.i

;-------------------------------------------------------------------

	.DATA?

ifdef DEBUG
sendStack	word		100 dup(?)				; room for 50 levels of messaging
ssEnd			label		word
ssPtr			word		?
endif

;-------------------------------------------------------------------

	.CODE		PM_TEXT

;---------------------------------------------------------------------------
; GetProperty	return the value of a property for an object
;
; entry:	obj		= address of object
;			property	= property number
; exit:	ax			= property value, or aborts if property is invalid
;---------------------------------------------------------------------------
GetProperty	proc	uses di, obj:ptr Obj, property:word

	;Get a pointer to the property.
	mov	bx, obj
	mov	ax, property

ifdef PROFILER
   add	ssPtr, 4
   mov	di, ssPtr
   mov	[di+_off], bx
   mov	[di+_seg], ax
endif

	GetPropAddrMacro
	
	.if !zero?
		mov	ax, [bx]
	.else
		invoke PError, si, bp, E_INVALID_PROPERTY, ax
	.endif

ifdef PROFILER
   sub	ssPtr, 4
endif

	ret

GetProperty	endp

;---------------------------------------------------------------------------
; SetProperty	set the value of a property for an object
;
; entry:	obj		= address of object
;			property	= property number
;			value		= property value
; exit:	none, aborts if property is invalid
;---------------------------------------------------------------------------
SetProperty	proc	uses di, obj:ptr Obj, property:word, value:word

	mov	bx, obj
	mov	ax, property

ifdef PROFILER
   add	ssPtr, 4
   mov	di, ssPtr
   mov	[di+_off], bx
   mov	[di+_seg], ax
endif

	GetPropAddrMacro

	.if !zero?
		mov	ax, value
		mov	[bx], ax
	.else
		invoke PError, si, bp, E_INVALID_PROPERTY, ax
	.endif

ifdef PROFILER
   sub	ssPtr, 4
endif

	ret

SetProperty	endp

;---------------------------------------------------------------------------
; GetPropAddr	gets the address of an object's property
;
; entry:	obj		= address of object
;			property	= property number
; exit:	ax			= address of property, aborts if property is invalid
;---------------------------------------------------------------------------
GetPropAddr	proc	uses di, obj:ptr Obj, property:word

	mov		bx, obj
	mov		ax, property

ifdef PROFILER
   add	ssPtr, 4
   mov	di, ssPtr
   mov	[di+_off], bx
   mov	[di+_seg], ax
endif

	GetPropAddrMacro
   mov	ax, bx

ifdef PROFILER
   sub	ssPtr, 4
endif

	ret

GetPropAddr	endp

NotObject:
	;A message was passed to a non-object.  Complain and croak.
	invoke	PError, si, bp, E_NOT_OBJECT, bx

QuickMessage proc

assume	bx:ptr Obj

ifdef DEBUG
   push  thisScript
endif
   push  scriptHandle
   push  es
	mov	ax, ds
	mov	es, ax
	;Check to see that we're really pointed at an object.
	cmp	[bx].oObjID, OBJID
	jnz	NotObject
ifdef DEBUG
	;Put the object which we're sending to on the send stack.
	add	ssPtr, 4
	mov	di, ssPtr
	.if	di < offset DGROUP:ssEnd
		; if we are out of send stack we will not write the information
		; debug.c will be aware of this
		mov	[di], bx
	.endif
endif
	;Set new locals.
	push	localVar
	mov	di, [bx].oScript
	mov	ax, (scriptNode ptr [di]).sVars
assume	bx:nothing
	mov	localVar, ax

	;Save the ip and the pointer to the parameter stack.
	push	si
	push	parmVar

	;RPoint to the messages on the parameter stack.
	mov	si, bp
	sub	si, dx
	push	si		                  			;save pointer to top of parameters
	add	si, 2

MessageLoop:
	;Check for completion of the send (no more messages).
	test	dx, dx
   jnz   MoreMessages

MessageDone:
	pop	bp		         						;restore pseudo-stack
	pop	parmVar
	pop	si
	pop	localVar
	pop   es          							;old code segment
   pop   bx          							;script node
ifdef DEBUG
   pop   thisScript
	sub	ssPtr, 4
endif
   mov   scriptHandle, bx
   mov   cx, (hunkPtr ptr [bx]).hSegment
   jcxz  MessageExit
ifdef DEBUG
   mov   defaultES, FALSE
endif
   mov   es, cx
	ret

MessageExit:
   call  FAR PTR NeedRetOpcode
	ret

MoreMessages:
	;Get the message selector and move the parameter pointer
	;past it.
   mov   ax, [si]                   ;selector in ax
   inc   si
   inc   si
	;Put this selector on the send stack.
ifdef DEBUG
	mov	di, ssPtr
	; if we are out of send stack we will not write the information
	; debug.c will be aware of this
	.if	di < offset DGROUP:ssEnd
		mov	[di+_seg], ax
	   mov   lastSp, 0   ;force update of sendStack
	.endif
endif

;***************************************************************************
;This may be a good place for speedup by changing both the compiler and this
;code.  Look into whether this loop would be faster if number of parameters
;or restArgs were number of parms/args or number of bytes.
;***************************************************************************
	;Get the number of bytes of arguments for this message and
	;adjust the number of bytes of parameters remaining accordingly.
	mov	cx, [si]			;number of parameters
	shl	cx, 1          ;bytes of parameters
	add	cx, restArgs
	sub	dx, cx         ;update byte count of parameteres on stack
	sub	dx, 4				;account for selector and number of parms
	mov	di, cx
	shr	di, 1
	mov	[si], di
	;Set up the new parameter variables and then 
   ;point past the parameters to this send.
	mov	parmVar, si
	add	si, cx
	add	si, 2
	;Save important stuff.
	push	dx                					;number of bytes of messages remaining
	push	si                					;pointer to parameter area
	push	bx                					;object
   push  localVar          					;local variables for this object

DoMessage:
	;Input:
	;	ax = message selector
	;	bx = pointer to object
ifdef	PROFILER
	.if traceOn == 1
		call	DoTrace
	.endif
endif
   shr   cx, 1
	mov	si, cx	   		   				;save the number of parameters

	;First, search the property dictionary for this selector.
assume	bx:ptr Obj
	mov	di, [bx].oClassScript				;get a pointer to the script node for the object's class
   mov   di, (scriptNode ptr [di]).sHunk	;get the handle of the hunk resource from the script node
   mov   cx, (hunkPtr ptr [di]).hSegment	;get the segment of the hunk resource from the handle
   mov   es, cx
   mov   di, [bx].oPropDict					;get the offset within the hunk resource to property dictionary
	mov	cx, [bx].oSize			      		;get number of properties in the dictionary
	mov	dx, cx
	repne	scasw
	.if zero?
	   ;Convert cx to word index into properties.
      inc   cx
      sub   dx,cx
      shl   dx,1
      add   bx,dx                   		;point bx at the property slot
		.if	!si
		   ;A query -- load value into accumulator.
		   mov	ax, word ptr [bx]
		   mov	acc, ax
	   .else
		   ;Not a query -- set the property.
		   mov	di, parmVar
		   mov	ax, [di+2]
		   mov	word ptr [bx], ax
		.endif
		mov	restArgs, 0
	   jmp	SendDone
		.endif

	;Not a property.  Search up through the method dictionary hierarchy
	;for the selector.
   mov   di, [bx].oScript						;get pointer to script node for this object
	.while TRUE
ifdef DEBUG
      mov   cx, (scriptNode ptr [di]).sKey  ;update current script number
      mov   thisScript, cx
endif
      mov   di, (scriptNode ptr [di]).sHunk	;get handle of hunk resource for this class
      mov   scriptHandle, di					;update current code handle
      mov   cx, (hunkPtr ptr [di]).hSegment	;get segment address of the hunk resource
      mov   es, cx

      mov   si, [bx].oMethDict				;get offset in hunk resource of method dictionary
		mov	cx, es:[si]     					;get number of selectors
		inc	si
		inc	si
		mov	dx, 4									;offset between method selectors in dictionary
		.if	cx
			.repeat
				.if	ax == es:[si]				;is this the selector?
					;Found the selector -- next word is method offset
					mov	si, es:[si+2]
					push	tempVar
					mov	restArgs, 0
					call	ExecuteCode
					pop	tempVar
					jmp   SendDone
					.endif
				add	si, dx						;point to the next method selector
			.untilcxz
		.endif

		;The selector is not a method for this class/object.
		;Search its superclass.
      mov   bx, [bx].oSuper					;get pointer to superclass
		or		bx, bx								;if no superclass, its an invalid method
		jz		BadSelector

		.if	[bx].oObjID != OBJID				;does this pointer point to a valid object?
			jmp	NotObject
			.endif

      mov   di, [bx].oScript					;get pointer to script node for this class
	   mov	dx, (scriptNode ptr [di]).sVars	;get pointer to variables in class' scope
	   mov	localVar, dx
	.endw



SendDone:
	;Recover important quantities and continue.
   pop   localVar          					;local variables for this object
	pop	bx                					;object
	pop	si                					;pointer to parameter area
	pop	dx                					;number of bytes of messages remaining
	jmp	MessageLoop

BadSelector:
	;Oops -- no such selector.  Raise an error
	mov	bx, object
	invoke	PError, si, bp, E_BAD_SELECTOR, bx, ax

assume	bx:nothing

QuickMessage	endp

Messager	proc	
;Send messages to the current object.
;Input:
;	bx = current object (pointer to property/method slots)
;
;Register usage:
;	ax = message selector
;	dx = number of bytes of parameters passed
;	si = pointer to parameter stack

	;Get the number of bytes of parameters passed, put in the dx.
	getbyteIndx
	add   dx, restArgs
	call	QuickMessage
	ret

Messager	endp

InvokeMethod	proc	uses si di, obj:ptr Obj, selector:word, n:word, args:word
;Invoke an object method from the kernel.  'obj' is the object to send the
;'selector' message.  'n' is the number of arguments to the method, 'args'
;is the start of the argument list.

	;Get all arguments in registers, as we're about to change to
	;the pmachine stack frame.
	mov   bx, obj
	mov	cx, n
	mov	di, selector
	lea	si, args

	;Set this as the current object.
	push	object
	mov	object, bx

	;Compute the number of bytes that will be put on the pmachine stack.
	mov	dx, cx
	add	dx, 2
	shl	dx, 1

	;Set the bp to the pmachine stack and push the arguments on it.
	mov	bp, pmsp
	ppush	di			;push selector
	ppush	cx			;push number of args
	.if	cx
		.repeat
			lodsw
			ppush		ax	;push an argument
		.untilcxz
	.endif

	;Send the message
	call		QuickMessage
	pop		object

	;Return the method result.
	mov		ax, acc
	mov		pmsp, bp
	ret

InvokeMethod	endp

	end

