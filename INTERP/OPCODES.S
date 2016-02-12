;----------------------------------------------------------------------
;OPCODES.S
;
;The opcodes for the pmachine.
;
;----------------------------------------------------------------------

include	macro.i

include debug.i
include kerndisp.i
include object.i
include pmachine.i

	.DATA

opTbl	label	word
	word	OP_bnot		    ;00
	word	BadOp           ;01
	word	OP_add          ;02
	word	BadOp           ;03
	word	OP_sub          ;04
	word	BadOp           ;05
	word	OP_mul          ;06
	word	BadOp           ;07
	word	OP_div          ;08
	word	BadOp           ;09
	word	OP_mod          ;0A
	word	BadOp           ;0B
	word	OP_shr          ;0C
	word	BadOp           ;0D
	word	OP_shl          ;0E
	word	BadOp           ;0F
	word	OP_xor          ;10
	word	BadOp           ;11
	word	OP_and          ;12
	word	BadOp           ;13
	word	OP_or			    ;14
	word	BadOp           ;15
	word	OP_neg          ;16
	word	BadOp           ;17
	word	OP_not          ;18
	word	BadOp           ;19
	word	OP_eq           ;1A
	word	BadOp           ;1B
	word	OP_ne           ;1C
	word	BadOp           ;1D
	word	OP_gt           ;1E
	word	BadOp           ;1F
	word	OP_ge           ;20
	word	BadOp           ;21
	word	OP_lt           ;22
	word	BadOp           ;23
	word	OP_le           ;24
	word	BadOp           ;25
	word	OP_ugt          ;26
	word	BadOp           ;27
	word	OP_uge		    ;28
	word	BadOp           ;29
	word	OP_ult          ;2A
	word	BadOp           ;2B
	word	OP_ule          ;2C
	word	BadOp           ;2D
	word	OP_bt_TWO       ;2E
	word	OP_bt_ONE       ;2F
	word	OP_bnt_TWO      ;30
	word	OP_bnt_ONE      ;31
	word	OP_jmp_TWO      ;32
	word	OP_jmp_ONE      ;33
	word	OP_loadi_TWO    ;34
	word	OP_loadi_ONE    ;35
	word	OP_push         ;36
	word	BadOp           ;37
	word	OP_pushi_TWO    ;38
	word	OP_pushi_ONE    ;39
	word	OP_toss         ;3A
	word	BadOp           ;3B
	word	OP_dup		    ;3C
	word	BadOp           ;3D
	word	OP_link_TWO     ;3E
	word	OP_link_ONE     ;3F
	word	OP_call_THREE   ;40
	word	OP_call_TWO     ;41
	word	OP_callk_THREE  ;42
	word	OP_callk_TWO    ;43
	word	OP_callb_THREE  ;44
	word	OP_callb_TWO    ;45
	word	OP_calle_FOUR   ;46
	word	OP_calle_TWO    ;47
	word	OP_ret          ;48
	word	BadOp           ;49
	word	OP_send_ONE     ;4A
	word	BadOp           ;4B
	word	BadOp           ;4C
	word	BadOp           ;4D
	word	BadOp           ;4E
	word	BadOp           ;4F
	word	OP_class_TWO    ;50
	word	OP_class_ONE    ;51
	word	BadOp           ;52
	word	BadOp           ;53
	word	OP_self_TWO     ;54
	word	OP_self_ONE     ;55
	word	OP_super_THREE  ;56
	word	OP_super_TWO    ;57
	word	BadOp           ;58
	word	OP_rest_ONE     ;59
	word	OP_lea_FOUR     ;5A
	word	OP_lea_TWO      ;5B
	word	OP_selfID       ;5C
	word	BadOp           ;5D
	word	BadOp           ;5E
	word	BadOp           ;5F
	word	OP_pprev		    ;60
	word	BadOp           ;61
	word	OP_pToa_TWO	    ;62   ;load prop to acc
	word	OP_pToa_ONE	    ;63   ;load prop to acc
	word	OP_aTop_TWO	    ;64   ;store acc to prop
	word	OP_aTop_ONE	    ;65   ;store acc to prop
	word	OP_pTos_TWO	    ;66   ;load prop to stack
	word	OP_pTos_ONE	    ;67   ;load prop to stack
	word	OP_sTop_TWO	    ;68   ;store stack to prop
	word	OP_sTop_ONE	    ;69   ;store stack to prop
	word	OP_ipToa_TWO    ;6A   ;inc prop
	word	OP_ipToa_ONE    ;6B   ;inc prop
	word	OP_dpToa_TWO    ;6C   ;dec prop
	word	OP_dpToa_ONE    ;6D   ;dec prop
	word	OP_ipTos_TWO    ;6E   ;inc prop to stack
	word	OP_ipTos_ONE    ;6F   ;inc prop to stack
	word	OP_dpTos_TWO    ;70   ;dec prop to stack
	word	OP_dpTos_ONE    ;71   ;dec prop to stack
	word	OP_lofsa_TWO    ;72
	word	BadOp           ;73
	word	OP_lofss_TWO    ;74
	word	BadOp           ;75
	word	OP_push0        ;76
	word	BadOp           ;77
	word	OP_push1        ;78
	word	BadOp           ;79
	word	OP_push2        ;7A
	word	BadOp           ;7B
	word	OP_pushSelf     ;7C
	word	BadOp           ;7D
	word	BadOp           ;7E
	word	BadOp           ;7F
	word	OP_lag_TWO      ;80
	word	OP_lag_ONE      ;81
	word	OP_lal_TWO      ;82
	word	OP_lal_ONE      ;83
	word	OP_lat_TWO      ;84
	word	OP_lat_ONE      ;85
	word	OP_lap_TWO      ;86
	word	OP_lap_ONE      ;87
	word	OP_lsg_TWO      ;88
	word	OP_lsg_ONE      ;89
	word	OP_lsl_TWO		 ;8A
	word	OP_lsl_ONE	    ;8B
	word	OP_lst_TWO      ;8C
	word	OP_lst_ONE      ;8D
	word	OP_lsp_TWO      ;8E
	word	OP_lsp_ONE      ;8F
	word	OP_lagi_TWO     ;90
	word	OP_lagi_ONE     ;91
	word	OP_lali_TWO     ;92
	word	OP_lali_ONE     ;93
	word	OP_lati_TWO     ;94
	word	OP_lati_ONE     ;95
	word	OP_lapi_TWO     ;96
	word	OP_lapi_ONE     ;97
	word	OP_lsgi_TWO     ;98
	word	OP_lsgi_ONE     ;99
	word	OP_lsli_TWO     ;9A
	word	OP_lsli_ONE     ;9B
	word	OP_lsti_TWO     ;9C
	word	OP_lsti_ONE     ;9D
	word	OP_lspi_TWO		 ;9E
	word	OP_lspi_ONE		 ;9F
	word	OP_sag_TWO      ;A0
	word	OP_sag_ONE      ;A1
	word	OP_sal_TWO      ;A2
	word	OP_sal_ONE      ;A3
	word	OP_sat_TWO      ;A4
	word	OP_sat_ONE      ;A5
	word	OP_sap_TWO      ;A6
	word	OP_sap_ONE      ;A7
	word	OP_ssg_TWO      ;A8
	word	OP_ssg_ONE      ;A9
	word	OP_ssl_TWO      ;AA
	word	OP_ssl_ONE      ;AB
	word	OP_sst_TWO      ;AC
	word	OP_sst_ONE      ;AD
	word	OP_ssp_TWO      ;AE
	word	OP_ssp_ONE      ;AF
	word	OP_sagi_TWO     ;B0
	word	OP_sagi_ONE     ;B1
	word	OP_sali_TWO	    ;B2
	word	OP_sali_ONE		 ;B3
	word	OP_sati_TWO     ;B4
	word	OP_sati_ONE     ;B5
	word	OP_sapi_TWO     ;B6
	word	OP_sapi_ONE     ;B7
	word	OP_ssgi_TWO     ;B8
	word	OP_ssgi_ONE     ;B9
	word	OP_ssli_TWO     ;BA
	word	OP_ssli_ONE     ;BB
	word	OP_ssti_TWO     ;BC
	word	OP_ssti_ONE     ;BD
	word	OP_sspi_TWO     ;BE
	word	OP_sspi_ONE     ;BF
	word	OP_iag_TWO      ;C0
	word	OP_iag_ONE      ;C1
	word	OP_ial_TWO      ;C2
	word	OP_ial_ONE      ;C3
	word	OP_iat_TWO      ;C4
	word	OP_iat_ONE      ;C5
	word	OP_iap_TWO	    ;C6
	word	OP_iap_ONE	    ;C7
	word	OP_isg_TWO      ;C8
	word	OP_isg_ONE      ;C9
	word	OP_isl_TWO      ;CA
	word	OP_isl_ONE      ;CB
	word	OP_ist_TWO      ;CC
	word	OP_ist_ONE      ;CD
	word	OP_isp_TWO      ;CE
	word	OP_isp_ONE      ;CF
	word	OP_iagi_TWO     ;D0
	word	OP_iagi_ONE     ;D1
	word	OP_iali_TWO     ;D2
	word	OP_iali_ONE     ;D3
	word	OP_iati_TWO     ;D4
	word	OP_iati_ONE     ;D5
	word	OP_iapi_TWO     ;D6
	word	OP_iapi_ONE     ;D7
	word	OP_isgi_TWO     ;D8
	word	OP_isgi_ONE     ;D9
	word	OP_isli_TWO	    ;DA
	word	OP_isli_ONE	    ;DB
	word	OP_isti_TWO     ;DC
	word	OP_isti_ONE     ;DD
	word	OP_ispi_TWO     ;DE
	word	OP_ispi_ONE     ;DF
	word	OP_dag_TWO      ;E0
	word	OP_dag_ONE      ;E1
	word	OP_dal_TWO      ;E2
	word	OP_dal_ONE      ;E3
	word	OP_dat_TWO      ;E4
	word	OP_dat_ONE      ;E5
	word	OP_dap_TWO      ;E6
	word	OP_dap_ONE      ;E7
	word	OP_dsg_TWO      ;E8
	word	OP_dsg_ONE      ;E9
	word	OP_dsl_TWO      ;EA
	word	OP_dsl_ONE      ;EB
	word	OP_dst_TWO      ;EC
	word	OP_dst_ONE      ;ED
	word	OP_dsp_TWO	    ;EE
	word	OP_dsp_ONE	    ;EF
	word	OP_dagi_TWO     ;F0
	word	OP_dagi_ONE     ;F1
	word	OP_dali_TWO     ;F2
	word	OP_dali_ONE     ;F3
	word	OP_dati_TWO     ;F4
	word	OP_dati_ONE     ;F5
	word	OP_dapi_TWO     ;F6
	word	OP_dapi_ONE     ;F7
	word	OP_dsgi_TWO     ;F8
	word	OP_dsgi_ONE     ;F9
	word	OP_dsli_TWO     ;FA
	word	OP_dsli_ONE     ;FB
	word	OP_dsti_TWO     ;FC
	word	OP_dsti_ONE     ;FD
	word	OP_dspi_TWO     ;FE
	word	OP_dspi_ONE     ;FF
                      ;100

;-------------------------------------------------------------------

	.CODE		PM_TEXT


ExecuteCode	proc	
;On entry:
;	es:si -> code to execute.

   mov   bl, es:[si]          ;Get the opcode and dispatch on it
   inc   si
   xor   bh,bh
   shl   bx,1
	jmp	opTbl[bx]   			;go do it

ExecuteCode	endp



OP_rest_ONE:
;Add the 'rest' of the current stack frame to the parameters which are
;already on the stack.

	mov	bx, parmVar	;get a pointer to the parameters
	mov	cx, [bx]			;get number of parameters in current frame
	getbyte					;get number of parameter to start with
	sub	cx, ax			;number of parameters to put on stack
	inc	cx
	.if	sign?
		xor	cx, cx
	.endif
   add	restArgs, cx	;tell others how to adjust stack frame
   add	restArgs, cx	;tell others how to adjust stack frame
	shl	ax, 1				;offset of starting parameter
	add	bx, ax			;pointer to starting parameter
	.if	cx
		.repeat
			mov	ax, [bx]	;get parameter
			add	bx, 2
			ppush	ax			;put it on the stack
		.untilcxz
	.endif
	NextOpCode

;Load the effective address of a variable into the acc.
	;Get the type of the variable in the bx, the number of the
	;variable in the dx, and convert the var number to an offset
	;in the variable space.
OP_lea_FOUR:
	getwordInbx
	getword
   jmp	OP_lea_continue
OP_lea_TWO:
	getbyteInbx
	getbyte
OP_lea_continue:
	mov	dx, ax
	shl	dx, 1						;*2 for offset
	;If the variable is indexed, add the index to the offset.
	;In this case, the index is in the acc.
	.if	bl & OP_INDEX
		mov	ax, acc
		shl	ax, 1
		add	dx, ax
	.endif
	;Get the effective address and put it in the acc.
	and	bx, OP_VAR				;get index into var base table
	mov	ax, theVars[bx]		;get the base of the var space
	add	ax, dx					;add the offset
	mov	acc, ax
	NextOpCode

;The following macro encapsulates the load offset operation:
LoadOfsWord	macro	dest
	getword
	storeax	dest
	NextOpCode
	endm

OP_lofsa_TWO:
	LoadOfsWord	acc

OP_lofss_TWO:
	LoadOfsWord	stack

OP_add:
;Add the top value of the stack to the acc.
	ppop	ax
	add	acc, ax
	NextOpCode

OP_sub:
;Subtract the acc from the top value on the stack.
	ppop	ax
	sub	ax, acc
	mov	acc, ax
	NextOpCode

OP_mul:
;Multiply the acc and the top value on the stack.
	ppop	ax
	mul	acc
	mov	acc, ax
	NextOpCode

OP_div:
;Divide the top value on the stack by the acc.
	ppop	ax
	cwd
	cmp	word ptr acc, 0
	jz		ZeroDivide
	idiv	acc
	mov	acc, ax
	NextOpCode

OP_mod:
;Put S (mod acc) in the acc.
	ppop	ax
	cwd
	cmp	word ptr acc, 0
	.if	!zero?
		.if	sign?
			neg	acc								;insure modulo positive
		.endif
		idiv	acc
		.if	dx & 8000h   						;test for neg case
			add	acc, dx	  	      			;make 0 <= modulo < (modulo-1)
			NextOpCode
		.else
			mov	acc, dx
			NextOpCode
		.endif
	.endif

ZeroDivide:
	invoke	PError, si, bp, E_ZERO_MODULO

OP_shr:
;Shift the value on the stack right by the amount in the acc.
	ppop	ax
	mov	cx, acc
	shr	ax, cl
	mov	acc, ax
	NextOpCode

OP_shl:
;Shift the value on the stack left by the amount in the acc.
	ppop	ax
	mov	cx, acc
	shl	ax, cl
	mov	acc, ax
	NextOpCode

OP_xor:
;Xor the value on the stack with that in the acc.
	ppop	ax
	xor	acc, ax
	NextOpCode

OP_and:
;And the value on the stack with that in the acc.
	ppop	ax
	and	acc, ax
	NextOpCode

OP_or:
;Or the value on the stack with that in the acc.
	ppop	ax
	or		acc, ax
	NextOpCode

OP_neg:
;Negate the value in the acc.
	neg	acc
	NextOpCode

OP_not:
;Do a logical not on the value in the acc.
	.if	acc
		mov	acc, 0
	.else
		mov	acc, 1
	.endif
	NextOpCode

OP_bnot:
;Do a bitwise not of the acc.
	xor	acc, 0ffffh
	NextOpCode

;The following comparison operators compare the top value on the stack with
;the acc, replace the top of the stack with the acc, and replace the acc
;with 1 or 0 depending on the outcome of the test.

;Since, in all likelihood, there will be more false tests than true tests,
;these routines are optimized for a false result.  They jump to common
;code to handle the true case, execute the false case in-line by falling
;through.

;The following macro encapsulates the comparison opcodes:

Comp	macro		cond
	mov	cx, acc
	cmp	cx, [bp]
if	($-True1+1) LT 127
	j&cond	True1
else
	j&cond	True2
endif
	sub	bp, 2
	mov	acc, 0
	NextOpCode
	endm

True1:
	sub	bp, 2
	mov	acc, 1
	NextOpCode

OP_eq:
;Test for equality.
	Comp	z

OP_ne:
;Test for inequality.
	Comp	nz

OP_gt:
;Is the stack value > acc?   (Signed)
	Comp	l

OP_ge:
;Is the stack value >= acc?   (Signed)
	Comp	le

OP_lt:
;Is the stack value < acc?   (Signed)
	Comp	g

OP_le:
;Is the stack value <= acc?   (Signed)
	Comp	ge

True2:
	sub	bp, 2
	mov	acc, 1
	NextOpCode

OP_ugt:
;Is the stack value > acc?   (Unsigned)
	Comp	b

OP_uge:
;Is the stack value >= acc?   (Unsigned)
	Comp	be

OP_ult:
;Is the stack value < acc?   (Unsigned)
	Comp	a

OP_ule:
;Is the stack value <= acc?   (Unsigned)
	Comp	ae

;The following branch opcodes are optimized for taking the branch
;(and within this catagory, taking a short branch), on the assumption
;that we will generally be skipping over code, not executing it.

;The actual branching is carried out by the following macro:

BranchWord	macro
	getword
	add	si, ax
	NextOpCode
	endm

BranchByte	macro
	getsbyte					;get the branch offset
	add	si, ax			;add offset to ip
	NextOpCode
	endm

;Add the following byte/word to the current scan pointer (the si)
;if acc is true.
OP_bt_ONE:
	cmp	acc, 0
	jz		Skip1		;skip over the branch offset
	BranchByte
OP_bt_TWO:
	cmp	acc, 0
	jz		Skip2		;skip over the branch offset
	BranchWord

;Add the following byte/word to the current scan pointer (the si)
;if acc is false.
OP_bnt_ONE:
	cmp	acc, 0
	jnz	Skip1		;skip over the branch offset
	BranchByte
OP_bnt_TWO:
	cmp	acc, 0
	jnz	Skip2		;skip over the branch offset
	BranchWord

Skip1:
;Skip over a branch offset.
	inc	si
	NextOpCode

Skip2:
;Skip over a branch offset.
	inc	si
	inc	si
	NextOpCode

;Unconditional branch.
OP_jmp_ONE:
	BranchByte
OP_jmp_TWO:
	BranchWord

;The following macro encapsulates the immediate operations:

;Load an immediate value into dest.
LoadIWord	macro	dest
	getword
	storeax	dest
	NextOpCode
	endm

LoadIByte	macro	dest
	getsbyte
	storeax	dest
	NextOpCode
	endm

OP_loadi_ONE:
	LoadIByte	acc
OP_loadi_TWO:
	LoadIWord	acc

OP_pushi_ONE:
	LoadIByte	stack
OP_pushi_TWO:
	LoadIWord	stack

;The following macro encapsulates the pushes of special values.

;Load an immediate value into dest.
PushSpec	macro	src, dest
	mov	ax, src
	storeax	dest
	NextOpCode
	endm

OP_push0:
	PushSpec	0

OP_push1:
	PushSpec	1

OP_push2:
	PushSpec	2

OP_pushSelf:
	PushSpec	object

OP_push:
;Push the value in the acc on the stack.
	mov	ax, acc
	ppush	ax
	NextOpCode

OP_toss:
;Pop the stack and discard the value.
	sub	bp, 2
	NextOpCode

OP_dup:
;Duplicate the current top value on the stack.
	peek	ax
	ppush	ax
	NextOpCode

;Link to a procedure by creating a temporary variable space.  This is done
;by decreasing both the sp and _tempVar by the amount in the following
;word/byte.
OP_link_ONE:
	getbyte
   jmp   SHORT OP_link_continue
OP_link_TWO:
	getword
OP_link_continue:
	mov	tempVar, bp
	add	tempVar, 2
	shl	ax, 1
	add	bp, ax
	cmp	bp, pStackEnd
	jae	StackOverflow
	NextOpCode

StackOverflow:
	invoke	PError, si, bp, E_STACK_BLOWN

;Make a call to a procedure in the current module.

OP_call_THREE:
   getwordInbx       ;put offset in bx
   jmp   SHORT OP_call_continue
OP_call_TWO:
	getsbyte          ;put offset in ax
	mov	bx, ax		;put offset in bx
OP_call_continue:
	getbyte
	push	si				;save ip
	add	si, bx		;point to procedure
	call	near ptr DoCall		;transfer control to the procedure
	pop	si
	NextOpCode

;Call a kernel routine.
OP_callk_TWO:
   ;save instruction pointer for disposescript in kernel
   mov   WORD PTR thisIP, si
   mov   WORD PTR thisIP+2, es
	getbyteInbx
   jmp   SHORT OP_callk_continue
OP_callk_THREE:
   ;save instruction pointer for disposescript in kernel
   mov   WORD PTR thisIP, si
   mov   WORD PTR thisIP+2, es
	getwordInbx
OP_callk_continue:
	;Get the kernel dispatch number and check that it is valid.
	.if	bx <= kernelMax
		push  pmsp
		mov	pmsp, bp
		shl	bx, 1			;bx = index into dispatch table
		shl	bx, 1
		;Set up the parameter pointer for the call.
		getbyte				;get number of parameters
		sub	bp, ax		;point to top of parameter space
   	mov	ax, restArgs
		sub	bp, ax
		shr	ax, 1
		add	[bp], ax
   	mov	restArgs, 0
		;Save the instruction and stack pointers, in case
		;the kernel routine (e.g. SaveGame) needs them.
		mov	ip, si
		;Dispatch to the routine.
      mov   ax, es
		push  es
		invoke	KernelCall ptr kernelDispTbl[bx], bp
		sub	bp, 2
      pop   es
		pop	pmsp

		UpdateScriptSeg

		NextOpCode
	.endif
	invoke	PError, si, bp, E_BAD_KERNAL, bx

OP_callb_TWO:
;Call a procedure in the base script.
	;Get script number in cx, entry number in bx, number of                     *
	;parameters in ax.                                                          *
	getbyteInbx
	xor	cx, cx			;base script
	getbyte
	call	Dispatch
	NextOpCode

OP_callb_THREE:
   getwordInbx
	xor	cx, cx			;base script
	getbyte
	call	Dispatch
	NextOpCode

OP_calle_TWO:
;Call an external script.
	;Get script number in cx, entry number in bx, number of
	;parameters in ax.
	getbyte		   		;get script number
	mov	cx, ax
	getbyteInbx				;get entry number
	getbyte
	call	Dispatch
	NextOpCode

OP_calle_FOUR:
;Call an external script.
	;Get script number in cx, entry number in bx, number of
	;parameters in ax.
	getword				   ;get module number
	mov	cx, ax
	getwordInbx				;get entry number
	getbyte
	call	Dispatch
	NextOpCode

OP_ret:
;Return from a call, dispatch, etc.
	retf

OP_send_ONE:
;Send messages to an object whose ID is in the _acc.
	mov	bx, acc		;get object address
	push	object
	mov	object, bx
	call	Messager
	pop	object
	NextOpCode

;Send to a class address based on the class number.
OP_super_TWO:
	getbyte		   		;get script number
   jmp   SHORT OP_super_continue
OP_super_THREE:
	getword
OP_super_continue:
	shl	ax, 1
	shl	ax, 1
	mov	bx, classTbl
	add	bx, ax
	mov	ax, [bx]
	.if	ax
SuperMessage:
		mov	bx, ax
		call	Messager
		NextOpCode
	.endif
	mov	ax, [bx+2]	;get script #
	call 	near ptr GetNewScript
	mov	ax, [bx]
	test	ax, ax
	jnz	SuperMessage
	sub	bx, classTbl
	shr	bx, 1
	shr	bx, 1
	invoke	PError, si, bp, E_LOAD_CLASS, bx

;Get a class address based on the class number.
OP_class_ONE:
	getbyte
   jmp   SHORT OP_class_continue
OP_class_TWO:
	getword
OP_class_continue:
	shl	ax, 1
	shl	ax, 1
	mov	bx, classTbl
	add	bx, ax
	mov	ax, [bx]
	.if	ax
ClassExit:
		mov	acc, ax
		NextOpCode
	.endif
	mov	ax, [bx+2]	;get script #
	call 	near ptr GetNewScript
	mov	ax, [bx]
	test	ax, ax
	jnz	ClassExit
	sub	bx, classTbl
	shr	bx, 1
	shr	bx, 1
	invoke	PError, si, bp, E_LOAD_CLASS, bx


OP_selfID:
;Return the address of the current object in the acc.
	mov	ax, object
	mov	acc, ax
	NextOpCode

OP_self_ONE:
OP_self_TWO:
;_Send to current object.
   mov	bx, object
	call	Messager
	NextOpCode

OP_pprev:
;Push previous value of _acc on the stack.  This is left in the cx after
;a comparison.
	ppush	cx
	NextOpCode

;The following macros encapsulates load operations:
LoadByte	macro	dest, src, indexed
ifb	<indexed>		    	;get variable number
	GetVarByte					;not indexed
else
	GetIndexByte				;indexed
endif
	add		bx, &src		;add base address of variable space
	mov		ax, [bx]			;get variable value
	storeax	dest
	NextOpCode
	endm

LoadWord	macro	dest, src, indexed
ifb	<indexed>			   ;get variable number
	GetVarWord					;not indexed
else
	GetIndexWord				;indexed
endif
	add		bx, &src		;add base address of variable space
	mov		ax, [bx]			;get variable value
	storeax	dest
	NextOpCode
	endm



OP_lag_ONE:
	LoadByte	acc, globalVar
OP_lag_TWO:
	LoadWord	acc, globalVar

OP_lal_ONE:
	LoadByte	acc, localVar
OP_lal_TWO:
	LoadWord	acc, localVar

OP_lat_ONE:
	LoadByte	acc, tempVar
OP_lat_TWO:
	LoadWord	acc, tempVar

OP_lap_ONE:
	LoadByte	acc, parmVar
OP_lap_TWO:
	LoadWord	acc, parmVar

OP_lsg_ONE:
	LoadByte	stack, globalVar
OP_lsg_TWO:
	LoadWord	stack, globalVar

OP_lsl_ONE:
	LoadByte	stack, localVar
OP_lsl_TWO:
	LoadWord	stack, localVar

OP_lst_ONE:
	LoadByte	stack, tempVar
OP_lst_TWO:
	LoadWord	stack, tempVar

OP_lsp_ONE:
	LoadByte	stack, parmVar
OP_lsp_TWO:
	LoadWord	stack, parmVar

OP_lagi_ONE:
	LoadByte	acc, globalVar, indexed
OP_lagi_TWO:
	LoadWord	acc, globalVar, indexed

OP_lali_ONE:
	LoadByte	acc, localVar, indexed
OP_lali_TWO:
	LoadWord	acc, localVar, indexed

OP_lati_ONE:
	LoadByte	acc, tempVar, indexed
OP_lati_TWO:
	LoadWord	acc, tempVar, indexed

OP_lapi_ONE:
	LoadByte	acc, parmVar, indexed
OP_lapi_TWO:
	LoadWord	acc, parmVar, indexed

OP_lsgi_ONE:
	LoadByte	stack, globalVar, indexed
OP_lsgi_TWO:
	LoadWord	stack, globalVar, indexed

OP_lsli_ONE:
	LoadByte	stack, localVar, indexed
OP_lsli_TWO:
	LoadWord	stack, localVar, indexed

OP_lsti_ONE:
	LoadByte	stack, tempVar, indexed
OP_lsti_TWO:
	LoadWord	stack, tempVar, indexed

OP_lspi_ONE:
	LoadByte	stack, parmVar, indexed
OP_lspi_TWO:
	LoadWord	stack, parmVar, indexed

;The following macros encapsulates the store operations:
StoreByte	macro	dest, src, indexed
ifidn	<src>, <acc>
	ifb	<indexed>			;get variable number
		GetVarByte					;not indexed
		add	bx, &dest		;add base address of variable space
		mov	ax, acc
	else
		GetIndexByte					;indexed
		add	bx, &dest
		ppop	ax
		mov	acc, ax
	endif
else
	ifb	<indexed>
		GetVarByte
	else
		GetIndexByte
	endif
		add	bx, &dest
		ppop	ax
endif
	mov	[bx], ax
	NextOpCode
	endm

StoreWord	macro	dest, src, indexed
ifidn	<src>, <acc>
	ifb	<indexed>			;get variable number
		GetVarWord					;not indexed
		add	bx, &dest		;add base address of variable space
		mov	ax, acc
	else
		GetIndexWord					;indexed
		add	bx, &dest
		ppop	ax
		mov	acc, ax
	endif
else
	ifb	<indexed>
		GetVarWord
	else
		GetIndexWord
	endif
		add	bx, &dest
		ppop	ax
endif
	mov	[bx], ax
	NextOpCode
	endm

OP_sag_ONE:
	StoreByte	globalVar, acc
OP_sag_TWO:
	StoreWord	globalVar, acc

OP_sal_ONE:
	StoreByte	localVar, acc
OP_sal_TWO:
	StoreWord	localVar, acc

OP_sat_ONE:
	StoreByte	tempVar, acc
OP_sat_TWO:
	StoreWord	tempVar, acc

OP_sap_ONE:
	StoreByte	parmVar, acc
OP_sap_TWO:
	StoreWord	parmVar, acc

OP_ssg_ONE:
	StoreByte	globalVar, stack
OP_ssg_TWO:
	StoreWord	globalVar, stack

OP_ssl_ONE:
	StoreByte	localVar, stack
OP_ssl_TWO:
	StoreWord	localVar, stack

OP_sst_ONE:
	StoreByte	tempVar, stack
OP_sst_TWO:
	StoreWord	tempVar, stack

OP_ssp_ONE:
	StoreByte	parmVar, stack
OP_ssp_TWO:
	StoreWord	parmVar, stack

OP_sagi_ONE:
	StoreByte	globalVar, acc, indexed
OP_sagi_TWO:
	StoreWord	globalVar, acc, indexed

OP_sali_ONE:
	StoreByte	localVar, acc, indexed
OP_sali_TWO:
	StoreWord	localVar, acc, indexed

OP_sati_ONE:
	StoreByte	tempVar, acc, indexed
OP_sati_TWO:
	StoreWord	tempVar, acc, indexed

OP_sapi_ONE:
	StoreByte	parmVar, acc, indexed
OP_sapi_TWO:
	StoreWord	parmVar, acc, indexed

OP_ssgi_ONE:
	StoreByte	globalVar, stack, indexed
OP_ssgi_TWO:
	StoreWord	globalVar, stack, indexed

OP_ssli_ONE:
	StoreByte	localVar, stack, indexed
OP_ssli_TWO:
	StoreWord	localVar, stack, indexed

OP_ssti_ONE:
	StoreByte	tempVar, stack, indexed
OP_ssti_TWO:
	StoreWord	tempVar, stack, indexed

OP_sspi_ONE:
	StoreByte	parmVar, stack, indexed
OP_sspi_TWO:
	StoreWord	parmVar, stack, indexed

;The following macro encapsulates the increment operations:
IncrementByte	macro	dest, src, indexed
ifb	<indexed>
	GetVarByte
else
	GetIndexByte
endif
	add		bx, &src
	inc		word ptr [bx]
	mov		ax, [bx]
	storeax	dest
	NextOpCode
	endm

IncrementWord	macro	dest, src, indexed
ifb	<indexed>
	GetVarWord
else
	GetIndexWord
endif
	add		bx, &src
	inc		word ptr [bx]
	mov		ax, [bx]
	storeax	dest
	NextOpCode
	endm

OP_iag_ONE:
	IncrementByte	acc, globalVar
OP_iag_TWO:
	IncrementWord	acc, globalVar

OP_ial_ONE:
	IncrementByte	acc, localVar
OP_ial_TWO:
	IncrementWord	acc, localVar

OP_iat_ONE:
	IncrementByte	acc, tempVar
OP_iat_TWO:
	IncrementWord	acc, tempVar

OP_iap_ONE:
	IncrementByte	acc, parmVar
OP_iap_TWO:
	IncrementWord	acc, parmVar

OP_isg_ONE:
	IncrementByte	stack, globalVar
OP_isg_TWO:
	IncrementWord	stack, globalVar

OP_isl_ONE:
	IncrementByte	stack, localVar
OP_isl_TWO:
	IncrementWord	stack, localVar

OP_ist_ONE:
	IncrementByte	stack, tempVar
OP_ist_TWO:
	IncrementWord	stack, tempVar

OP_isp_ONE:
	IncrementByte	stack, parmVar
OP_isp_TWO:
	IncrementWord	stack, parmVar

OP_iagi_ONE:
	IncrementByte	acc, globalVar, indexed
OP_iagi_TWO:
	IncrementWord	acc, globalVar, indexed

OP_iali_ONE:
	IncrementByte	acc, localVar, indexed
OP_iali_TWO:
	IncrementWord	acc, localVar, indexed

OP_iati_ONE:
	IncrementByte	acc, tempVar, indexed
OP_iati_TWO:
	IncrementWord	acc, tempVar, indexed

OP_iapi_ONE:
	IncrementByte	acc, parmVar, indexed
OP_iapi_TWO:
	IncrementWord	acc, parmVar, indexed

OP_isgi_ONE:
	IncrementByte	stack, globalVar, indexed
OP_isgi_TWO:
	IncrementWord	stack, globalVar, indexed

OP_isli_ONE:
	IncrementByte	stack, localVar, indexed
OP_isli_TWO:
	IncrementWord	stack, localVar, indexed

OP_isti_ONE:
	IncrementByte	stack, tempVar, indexed
OP_isti_TWO:
	IncrementWord	stack, tempVar, indexed

OP_ispi_ONE:
	IncrementByte	stack, parmVar, indexed
OP_ispi_TWO:
	IncrementWord	stack, parmVar, indexed

;The following macro encapsulates the decrement operations:
DecrementByte	macro	dest, src, indexed
ifb	<indexed>
	GetVarByte
else
	GetIndexByte
endif
	add		bx, &src
	dec		word ptr [bx]
	mov		ax, [bx]
	storeax	dest
	NextOpCode
	endm

DecrementWord	macro	dest, src, indexed
ifb	<indexed>
	GetVarWord
else
	GetIndexWord
endif
	add		bx, &src
	dec		word ptr [bx]
	mov		ax, [bx]
	storeax	dest
	NextOpCode
	endm

OP_dag_ONE:
	DecrementByte	acc, globalVar
OP_dag_TWO:
	DecrementWord	acc, globalVar

OP_dal_ONE:
	DecrementByte	acc, localVar
OP_dal_TWO:
	DecrementWord	acc, localVar

OP_dat_ONE:
	DecrementByte	acc, tempVar
OP_dat_TWO:
	DecrementWord	acc, tempVar

OP_dap_ONE:
	DecrementByte	acc, parmVar
OP_dap_TWO:
	DecrementWord	acc, parmVar

OP_dsg_ONE:
	DecrementByte	stack, globalVar
OP_dsg_TWO:
	DecrementWord	stack, globalVar

OP_dsl_ONE:
	DecrementByte	stack, localVar
OP_dsl_TWO:
	DecrementWord	stack, localVar

OP_dst_ONE:
	DecrementByte	stack, tempVar
OP_dst_TWO:
	DecrementWord	stack, tempVar

OP_dsp_ONE:
	DecrementByte	stack, parmVar
OP_dsp_TWO:
	DecrementWord	stack, parmVar

OP_dagi_ONE:
	DecrementByte	acc, globalVar, indexed
OP_dagi_TWO:
	DecrementWord	acc, globalVar, indexed

OP_dali_ONE:
	DecrementByte	acc, localVar, indexed
OP_dali_TWO:
	DecrementWord	acc, localVar, indexed

OP_dati_ONE:
	DecrementByte	acc, tempVar, indexed
OP_dati_TWO:
	DecrementWord	acc, tempVar, indexed

OP_dapi_ONE:
	DecrementByte	acc, parmVar, indexed
OP_dapi_TWO:
	DecrementWord	acc, parmVar, indexed

OP_dsgi_ONE:
	DecrementByte	stack, globalVar, indexed
OP_dsgi_TWO:
	DecrementWord	stack, globalVar, indexed

OP_dsli_ONE:
	DecrementByte	stack, localVar, indexed
OP_dsli_TWO:
	DecrementWord	stack, localVar, indexed

OP_dsti_ONE:
	DecrementByte	stack, tempVar, indexed
OP_dsti_TWO:
	DecrementWord	stack, tempVar, indexed

OP_dspi_ONE:
	DecrementByte	stack, parmVar, indexed
OP_dspi_TWO:
	DecrementWord	stack, parmVar, indexed

;These opcodes access properties of objects as quickly as possible.
OP_pToa_ONE:
;Load prop to acc
	GetIndexedPropPtrByte
	mov	ax, [bx]
	mov	acc, ax
	NextOpCode

OP_pToa_TWO:
;Load prop to acc
	GetIndexedPropPtrWord
	mov	ax, [bx]
	mov	acc, ax
	NextOpCode

OP_pTos_ONE:
;Load prop to stack
	GetIndexedPropPtrByte
	mov	ax, [bx]
	ppush	ax
	NextOpCode

OP_pTos_TWO:
;Load prop to stack
	GetIndexedPropPtrWord
	mov	ax, [bx]
	ppush	ax
	NextOpCode

OP_aTop_ONE:
;Store acc to prop
	GetIndexedPropPtrByte
	mov	ax, acc
	mov	[bx], ax
	NextOpCode

OP_aTop_TWO:
;Store acc to prop
	GetIndexedPropPtrWord
	mov	ax, acc
	mov	[bx], ax
	NextOpCode

OP_sTop_ONE:
;Store stack to prop
	GetIndexedPropPtrByte
	ppop	ax
	mov	[bx], ax
	NextOpCode

OP_sTop_TWO:
;Store stack to prop
	GetIndexedPropPtrWord
	ppop	ax
	mov	[bx], ax
	NextOpCode

OP_ipToa_ONE:
;Inc prop
	GetIndexedPropPtrByte
	inc	word ptr [bx]
	mov	ax, [bx]
	mov	acc, ax
	NextOpCode

OP_ipToa_TWO:
;Inc prop
	GetIndexedPropPtrWord
	inc	word ptr [bx]
	mov	ax, [bx]
	mov	acc, ax
	NextOpCode

OP_ipTos_ONE:
;Inc prop to stack
	GetIndexedPropPtrByte
	inc	word ptr [bx]
	mov	ax, [bx]
	ppush	ax
	NextOpCode

OP_ipTos_TWO:
;Inc prop to stack
	GetIndexedPropPtrWord
	inc	word ptr [bx]
	mov	ax, [bx]
	ppush	ax
	NextOpCode

OP_dpToa_ONE:
;Dec prop
	GetIndexedPropPtrByte
	dec	word ptr [bx]
	mov	ax, [bx]
	mov	acc, ax
	NextOpCode

OP_dpToa_TWO:
;Dec prop
	GetIndexedPropPtrWord
	dec	word ptr [bx]
	mov	ax, [bx]
	mov	acc, ax
	NextOpCode

OP_dpTos_ONE:
;Dec prop to stack
	GetIndexedPropPtrByte
	dec	word ptr [bx]
	mov	ax, [bx]
	ppush	ax
	NextOpCode

OP_dpTos_TWO:
;Dec prop to stack
	GetIndexedPropPtrWord
	dec	word ptr [bx]
	mov	ax, [bx]
	ppush	ax
	NextOpCode

BadOp:
   dec   si                   ;get back to opcode
	invoke	PError, si, bp, E_BAD_OPCODE, bx


	end


