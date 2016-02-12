;-------------------------------------------------------------------
;	pmachine.i
;	Definitions for the Script Pseudo-Machine.
;-------------------------------------------------------------------

ifndef PMACHINE_I
PMACHINE_I	equ	1

ifndef SCRIPT_I
include script.i
endif

ifndef OBJECT_I
include object.i
endif

hunkPtr     struc
	hOffset	word		?
	hSegment	word		?
hunkPtr     ends

;Definitions of some basic parameters of the system.
STACKSIZE	   equ	2000h
PSTACKSIZE	   equ	01400h	;in bytes
HEAPSIZE			equ	1000h		;segment size of heap (64K)

;The bit in the opcode which indicates whether the parameters are
;bytes or words.
OP_INDEX			equ	10h						;indexed op if set, non-indexed otw
OP_VAR			equ	06h						;mask for var type

; minimum acceptable address in heap where an object could reside
MINOBJECTADDR	equ   2000h

;Bits in the -info- property.
CLASSBIT			equ	8000h
CLONEBIT			equ	0001h
NODISPOSE		equ	0002h
NODISPLAY		equ	0004h						;don't display in ShowObj()

;The value in O_ID which signifies an object.
OBJID				equ	01234h

;Error numbers.
E_BAD_DISPATCH			equ   0
E_BAD_OPCODE			equ   1
E_BAD_KERNAL			equ   2
E_LOAD_CLASS			equ   3
E_NOT_OBJECT			equ   4
E_BAD_SELECTOR			equ   5
E_CANT_FIXUP			equ   6
E_ZERO_DIVIDE			equ   7
E_STACK_BLOWN			equ   8
E_ZERO_MODULO			equ   9
E_LEFT_CLONE			equ   10
E_VER_STAMP_MISMATCH		equ   11
E_PACKHANDLE_HEAP		equ   12
E_PACKHANDLE_HUNK		equ   13
E_PACKHANDLE_FAILURE		equ   14
E_ODD_HEAP_RETURNED		equ   15

externdef 	opTbl:word
externdef 	pmCode:FuncPtr
externdef 	pmCodeSegment:word
externdef 	gameStarted:bool
externdef 	theGame:ObjPtr
externdef 	ip:memptr
externdef 	pmsp:wordPtr
externdef 	acc:word
externdef 	pStack:wordPtr
externdef 	pStackEnd:wordPtr
externdef 	theVars:wordPtr
externdef 	globalVar:wordPtr
externdef 	localVar:wordPtr
externdef 	tempVar:wordPtr
externdef 	parmVar:wordPtr
externdef 	restArgs:word
externdef 	object:ObjPtr
externdef 	scriptHandle:Handle
externdef	numClasses:word
externdef	defaultES:bool
externdef	thisScript:word
externdef	thisIP:dword

PMachine						proto
Dispatch						proto
ExecuteCode					proto
GetDispatchAddr			proto	scrptNde:word, entry:word
GetNewScript				proto near
NeedRetOpcode				proto far
GetDispatchAddrInHeap	proto near
GetDispatchAddrInHunk	proto near
DoCall						proto near

;This is the macro which fetches and executes the next op-code
NextOpCode	macro
	mov   bl, es:[si]          ;get opcode
	inc   si
	xor   bh,bh
	shl   bx,1
	jmp	opTbl[bx]				;go do it
	endm

;This macro pushes a register on the pmachine stack.
ppush	macro	reg
	add	bp, 2
	mov	[bp], reg
	endm

;This macro pops the top of the pmachine stack into a register.
ppop	macro	reg
	mov	reg, [bp]
	sub	bp, 2
	endm

;This macro reads the value on the top of the pmachine stack.
peek	macro	reg
	mov	reg, [bp]
	endm

;This is a macro to encapsulate reading words from external files.
;Since these are currently written in 8086 native form, this is
;just a lodsw.
getword	macro
	mov   ax, es:[si]
	inc   si
	inc   si
	endm

getwordInbx	macro
	mov   bx, es:[si]
	inc   si
	inc   si
	endm

;A macro to read a byte (unsigned).
getbyte	macro
	mov   al, es:[si]
	inc   si
	xor	ah, ah
	endm

getbyteInbx	macro
	mov   bl, es:[si]
	inc   si
	xor	bh, bh
	endm

getbyteIndx	macro
	mov   dl, es:[si]
	inc   si
	xor	dh, dh
	endm

;A macro to read a byte (signed).
getsbyte	macro
	mov   al, es:[si]
	inc   si
	cbw
	endm

;Stores the ax either in the acc or on the stack, depending on the
;value of dest.
storeax	macro	dest
ifidn	<dest>,<acc>
	mov	acc, ax
else
	ppush	ax
endif
	endm

GetVarWord		macro
	getwordInbx
	shl	bx, 1
	endm

GetVarByte		macro
	getbyteInbx
	shl	bx, 1
	endm

GetIndexWord	macro
	getwordInbx
	add	bx, acc
	shl	bx, 1
	endm

GetIndexByte	macro
	getbyteInbx
	add	bx, acc
	shl	bx, 1
	endm

GetIndexedPropPtrWord	macro
;Get a pointer to a property in the current object which is specified as
;an index into the property storage of the object
	getwordInbx
	add	bx, object
	endm


GetIndexedPropPtrByte	macro
;Get a pointer to a property in the current object which is specified as
;an index into the property storage of the object
	getbyteInbx
	add	bx, object
	endm
	
;----------------------------------------------------------------------------
; UpdateScriptSeg		updates ES with current script's segment, in case it
;							moved, and if it was disposed, makes sure the next
;							opcode is a return
; entry:			nothing
; destroyed:	di, cx
; exit:			es	= segment of current script, or next opcode is an ret
;----------------------------------------------------------------------------
UpdateScriptSeg	macro

   mov   di, scriptHandle
	mov   cx, (hunkPtr ptr [di]).hSegment
	.if	cx
ifdef DEBUG
      mov   defaultES, FALSE
endif
      mov   es, cx
	.else
      call  NeedRetOpcode
	.endif

endm

endif

