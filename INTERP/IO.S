;-------------------------------------------------------------------
; io.s
;-------------------------------------------------------------------


include	macro.i
include	stdio.i
include	dos.i
include	io.i



	.CODE


lseek	proc	fd:word, ofs:dword, theOrg:word

;Input:
;       fd	file handle
;       ofs	double-word offset [ofsh, ofsl]
;       theorg  offset is from:
;               0 = offset from beginning of file
;               1 = current location + offset
;               2 = EOF + offset
;Output:
;       returns double-word file pointer


	call	ClearDosError
	mov	bx, fd
	mov	al, byte ptr theOrg
	mov	dx, word ptr ofs+_off
	mov	cx, word ptr ofs+_seg
	dos	dosLSeek
	.if	carry?
		mov	ax, -1
		mov	dx, ax
	.endif

	ret

lseek	endp


	end
