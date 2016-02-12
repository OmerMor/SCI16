;-------------------------------------------------------------------
;	macro.i
;
;	A collection of defines and macros used throughout the assembly code.
;	This also sets the model and processor types
;-------------------------------------------------------------------

ifndef	MACRO_I

MACRO_I	equ	1

.model	medium, c, os_dos
option	nokeyword:<loop>

;; Maybe someday we can do this...
;; .286

bool		typedef word
wordPtr	typedef ptr word
Func		typedef proto
FuncPtr	typedef ptr Func
HunkPtr	typedef far ptr byte
Handle	typedef ptr HunkPtr
memptr	typedef ptr byte

NULL		equ	00h
EOS		equ	00h
LF 		equ	0ah
CR	   	equ	0dh
ESCCHAR	equ	1bh
SPACE		equ	20h

TRUE		equ	1
FALSE		equ	0

;Offsets within a double-word of the segment/offset or high/low words
_off		equ	0
_ofs		equ	0
_seg		equ	2
lo 		equ	0
hi	   	equ	2

return	textequ	<ret>

;; Some macros to do the reverse of the les & lds op-codes.

ses	macro		var, reg
	mov	word ptr var[_ofs], reg
	mov	word ptr var[_seg], es
	endm

sds	macro		var, reg
	mov	word ptr var[_ofs], reg
	mov	word ptr var[_seg], ds
	endm

;;Macros to save and restore general-purpose register depend on processor type.

;if	@Cpu and 00000001y

pushall	macro
	pushf
	push	ax
	push	cx
	push	dx
	push	bx
	push	bp
	push	si
	push	di
	endm

popall	macro
	pop	di
	pop	si
	pop	bp
	pop	bx
	pop	dx
	pop	cx
	pop	ax
	popf
	endm

;else

;pushall	textequ	<pusha>
;popall	textequ	<popa>

;endif


bios	macro	interrupt, function
ifnb	<function>
	mov	ah, function
endif
	int	interrupt
	endm

dos	macro	function,subfunc
ifnb	<subfunc>
	mov	ax, (function shl 8) or subfunc
else
	mov	ah, function
endif
	int	21h
	endm


endif

