;-------------------------------------------------------------------
; FILEIO.S
; File open, close, and i/o routines.
; Modifications:  
;   3/12/90 Abdul.  Changed to use .MODEL directive and simplified
;                   segments.  Microsoft MASM 5.1.
;-------------------------------------------------------------------


;-------------------------------------------------------------------

include	macro.i

include	dos.i
include	io.i
include	stdio.i
include	string.i
include	fileio.i

;-------------------------------------------------------------------

NO_HANDLE	   equ	-1
ERROR_CODE	   equ	-1

;-------------------------------------------------------------------

	.DATA


criticalError	word	0
oldCritErr		dword	?


;-------------------------------------------------------------------

	.CODE


readfar	proc	fd:word, handle:word, n:word
;Read 'n' bytes from file 'fd' into the hunk pointed to by 'handle'.

	mov	bx, handle
	mov	dx, [bx]
	mov	ax, [bx+_seg]
	invoke	ReadDos, fd, ax::dx, n

	ret
readfar	endp


ReadDos	proc	uses ds di, fd:word, handle:far ptr byte, n:word

; Special dos read function to use real far pointer rather than the pointer
; through the handle.  This is necessary for the pk Explode function

	call	ClearDosError
	mov	bx, fd
	mov	cx, n
	mov	dx, word ptr handle + 2
	mov	ds, dx
	mov	dx, word ptr handle
	dos	dosRead
	.if	carry?
		xor	ax, ax
	.endif

	ret

ReadDos endp


WriteDos	proc uses ds fd:word, buffer:far ptr word, n:word
;Write 'n' bytes from file 'fd' into the far pointer 'buffer'.

	call	ClearDosError
	mov	bx, fd
	mov	cx, n
	lds	dx, buffer
	dos	dosWrite
	.if	carry?
		xor	ax, ax
	.endif

	ret

WriteDos endp


unlink	proc	fname:ptr byte
;Unlink the file 'fname'

	call	ClearDosError
	mov	dx, fname
	dos	dosUnlink
	.if	carry?
		xor	ax, ax
	.endif

	ret

unlink	endp



filelength	proc	fd:word
;Return the (long) length of the file corresponding to a handle.

	xor	bx, bx
	invoke	lseek, fd, 0, 2
	push	ax											;save returned values
	push	dx
	xor	bx, bx
	invoke	lseek, fd, 0, bx					;seek back to start of file
	pop	dx
	pop	ax

	ret
filelength	endp



getcwd	proc	uses si di, dirStr:ptr byte
;Put the current directory into the string pointed to by dirStr.
;Note that this must also contain the current device.

assume	di:ptr byte

	mov	di, ds
	mov	es, di

	mov	di, dirStr

	dos	dosCurDrive
	xor	ah, ah
	add	al, 'a'
	stosb
	mov	al, ':'
	stosb
	mov	al, '\'
	stosb

	xor	dl, dl		;default drive
	mov	si, di
	dos	dosCurDir

	;Replace the starting '\' with a '/' if a '/' appears in the pathname
	invoke	strlen, dirStr
	mov	cx, ax
	mov	al, '/'
	mov	di, dirStr
	repne	scasb
	.if	cx
		mov	di, dirStr
		mov	[di+2], '/'
	.endif		

	invoke	strlwr, dirStr
	mov	ax, dirStr

	ret

assume	di:nothing

getcwd	endp



firstfile	proc	fname:ptr byte, attr:word, DTA:ptr byte
;_Find the first file which matches 'firstName' with attribute 'firstAttr'.
;Put results in 'firstDTA'

	mov	dx, DTA
	dos	dosSetDTA
	mov	dx, fname
	mov	cx, attr
	dos	dosFirst
	.if	carry?
		xor	ax, ax
	.else
		mov	ax, 1
	.endif

	ret

firstfile	endp


nextfile	proc	DTA:ptr byte
;_Find the next file from search started using DTA
;Put results in DTA

	mov	dx, DTA
	dos	dosSetDTA
	dos	dosNext
	.if	carry?
		xor	ax, ax
	.else
		mov	ax, 1
	.endif

	ret

nextfile	endp

existsdriveAB	proc
;Check for the validity of drive a: or b: by checking
;if there is more than one floppy drive, and how it is assigned
;
;	input: dl=0 for drive a: check, 1 for drive b: check
;
;  returns: ax
;    ax =  0 ---- drive requested does not exist       
;    ax != 0 ---- drive requested exists

	int	dosEquipCfg
	push  bx

	;bit 0 indicates if there are floppy drive(s)
	mov	bx, ax
	and   ax, 1
	.if	!zero?
		; there are floppy drive(s) -- count of (floppy_drives -1)  in ax
		mov	ax, bx
		and   ax, 00c0h
		.if	!ax
			push	ds						;only 1 floppy drive
			mov	ax, 50h
			mov	ds, ax
			mov	bx, 04h
			mov	ax, 1
			and	al, [bx]				;if drive assigned as a: ax=0 else ax=1
			xor	al, dl				;if drive requested=drive assigned ax=FALSE
			xor	al, 1					;ax=TRUE if drive requested=drive assigned
			pop	ds
		.endif
	.endif
	pop   bx

	ret

existsdriveAB	endp


existdrive	proc	theDrive:word
;Check for the validity of a particular drive by switching to
;it and then back.

	call	ClearDosError

	;Get and save the current drive
	dos	dosCurDrive
	mov	cl, al

	;Try changing drives, and see if we get to the requested one.
	mov	dl, byte ptr theDrive
	or		dl, 20h		; convert to lower case
	sub	dl, 'a'

   ;If this is a: or b:, test if it exists
	.if	dl <= 1
      call	existsdriveAB
		.if	!ax
         ;no b: drive
		   xor	bx, bx
		   jmp	EDExit
	   .endif
	.endif

	dos	dosSetDrive
	dos	dosCurDrive			;see if we got there
	xor	bx, bx
	.if	al == dl
		inc	bx					;got there -- a valid drive
	.endif

	mov	dl, cl
	dos	dosSetDrive

EDExit:
	mov	ax, bx
	ret

existdrive	endp



ResetDisk	proc	
;Reset the disk subsystem.

	dos	dosResetDisk
	return

ResetDisk	endp



RGetFreeSpace	proc	drive:byte
;Get the amount of free space on the drive passed in 'drive'.
;If 'drive' is 0, use the default drive.

	call	ClearDosError

	mov	dl, drive
	.if	dl == ' '
		xor	dl, dl
	.else
		.if dl
			or		dl, 20h		; convert to lower case
			sub	dl, 'a'-1
		.endif
	.endif
	dos	dosFreeSpace
	.if	carry?
		xor	ax, ax
		xor	dx, dx
	.else
		xor	dx, dx
		.if	ax == -1
			xor	ax, ax	;invalid drive -- return no space
		.else
			dec	bx	;report one less cluster than is there - fudge factor
			mul	bx	;multiply available clusters by sectors/cluster
			mul	cx	;then by bytes/sector
		.endif
	.endif

	ret

RGetFreeSpace	endp


Exists	proc	fileSpec:ptr byte
;Routine provides a method to establish existence of
;a given file on a given device, WITHOUT alerting the user

	; replace existing critical error handler
	; save old one
	dos	dosGetVector, 24h
	mov	word ptr oldCritErr+_off, bx
	mov	word ptr oldCritErr+_seg, es

	; set new one
	push	ds
	mov	ax, cs
	mov	ds, ax
	lea	dx, MyCritErrorHandler
	dos	dosSetVector, 24h
	pop	ds

	;look to see if referencing drive a: or b:
	mov	bx, fileSpec
	mov	ax, word ptr [bx]
	.if	ax == ':a' || ax == ':A' || ax == ':b' || ax == ':B'
		xor	ah, ah								;get rid of colon
		or		al, 20h								;guarantee lowercase
		invoke existdrive, ax
	.endif

	.if	ax
		mov	dx, fileSpec
		dos	dosOpen, 0
		.if	carry?
			xor	ax, ax
		.else
			;file is open -- close it
			mov	bx, ax
			dos	dosClose, 0
			mov	ax, 1
		.endif
	.endif

	push	ax
	; now replace the old critical error handler
	push	ds
	mov	dx, word ptr oldCritErr+_off
	mov	ds, word ptr oldCritErr+_seg
	dos	dosSetVector, 24h
	pop	ds

	; return the result
	pop	ax
	ret

Exists	endp
	


MyCritErrorHandler	proc	far
;return an error to calling procedure.

	;Pop DOS's return address off the stack
	add	sp,6

	;Set the carry flag in the flags which will 
	;be returned to the caller
	mov	bp,sp
	mov	ax,[bp+22]								;get the original flags
	or		ax,1										;set the carry
	mov	[bp+22],ax								;and put the flags back

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

	iret

MyCritErrorHandler	endp


drivecheck	proc	drvStr:ptr byte
;Return TRUE if drvStr points to strings starting with either "a:" or "b:",
;FALSE otherwise.

	xor	ax, ax					;setup for FALSE return
	mov	bx, drvStr
	mov	bx, word ptr [bx]
	or		bl, 20h					; convert to lowercase
	.if	(bl == 'a' || bl == 'b') && bh == ':'
		inc	ax
	.endif

	return

drivecheck	endp

;---------------------------------------------------------------------------
; rename		rename a DOS file
;
; entry:	oldName	= old filename
;			newName	= new filename
; exit:	ax			= 0: success
;						  2: file not found
;						  3: path not found
;						  5: access denied
;						 11: not same device	
;---------------------------------------------------------------------------
rename	proc	oldName:ptr byte, newName:ptr byte
	mov	dx, oldName		; ds:dx = old name
	push	ds
	pop	es
	mov	di, newName		; es:di = new name

	dos	dosRename
	jc		@F
	xor	ax, ax
@@:
	ret
rename	endp

	end
