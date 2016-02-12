;-------------------------------------------------------------------
;	stdlib.i
;-------------------------------------------------------------------

ifndef STDLIB_I

STDLIB_I		equ	1

atoi		proto	s:ptr byte
itoa		proto	n:word, s:ptr byte, radix:word
ultoa		proto	n:word, s:ptr byte, radix:word

endif
