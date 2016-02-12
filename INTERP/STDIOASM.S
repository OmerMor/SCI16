;-------------------------------------------------------------------
; STDIOASM.S
; File open, close, and i/o routines.
; Modifications:  
;   3/12/90 Abdul.  Changed to use .MODEL directive and simplified
;                   segments.  Microsoft MASM 5.1.
;-------------------------------------------------------------------

include	macro.i
include	stdio.i
include	dos.i
include	string.i
include	fileio.i

;-------------------------------------------------------------------


NO_HANDLE	   equ	-1
ERROR_CODE	   equ	-1

;-------------------------------------------------------------------


	.CODE


ClearDosError	proc	

	push	ds
	mov	ax, @data
	mov	ds, ax
	mov	criticalError, 0
	pop	ds
	return

ClearDosError	endp



creat	proc	fname:ptr byte, mode:word	
;Create a file with name pointed to by 'fname' with attributes set in 'mode'.

	call	ClearDosError
	mov	dx, fname
	mov	cx, mode
	dos	dosCreat
	.if	carry?
		mov	ax, NO_HANDLE
	.endif
	ret

creat	endp



open	proc	fname:ptr byte, mode:word
;Open the file with name 'fname'.
;The access is defined by 'mode':
;       0 = _read
;       1 = _write
;       2 = _read/_write

	call	ClearDosError
	mov	dx, fname
	mov	al, byte ptr mode
	dos	dosOpen
	.if	carry?
		mov	ax, NO_HANDLE
  	.endif
	ret

open	endp



read	proc	fd:word, buf:ptr byte, n:word
;Read 'n' bytes from file 'fd' into 'buf'.

	call	ClearDosError
	mov	bx, fd
	mov	cx, n
	mov	dx, buf
	dos	dosRead
	.if	carry?
		xor	ax, ax
	.endif
	ret

read	endp



write	proc	fd:word, buf:ptr byte, n:word
;Write 'n' bytes from 'buf' to the file 'fd'.

	call	ClearDosError
	mov	bx, fd
	mov	cx, n
	mov	dx, buf
	dos	dosWrite
	.if	carry?
		xor	ax, ax
	.endif
	ret

write	endp



close	proc	fd:word
;Close the file 'fd'.

	call	ClearDosError
	mov	bx, fd
	dos	dosClose
	.if	carry?
		mov	ax, 1
	.else
		mov	ax, 0
	.endif
	ret

close	endp

	end
