;	setjmp.i

ifndef SETJMP_I
SETJMP_H	equ 1

jmp_buf	struc
	jmp_seg	word	?
	jmp_off	word	?
	jmp_si	word	?
	jmp_di	word	?
	jmp_bp	word	?
	jmp_sp	word	?
	jmp_es	word	?
jmp_buf	ends

setjmp	proto buf: jmp_buf
longjmp	proto buf: jmp_buf, code: word

endif
