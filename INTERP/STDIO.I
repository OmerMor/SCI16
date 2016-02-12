;-------------------------------------------------------------------
;	stdio.i
;-------------------------------------------------------------------

ifndef	STDIO_I

STDIO_I	equ	1

creat				proto	fname:ptr byte, mode:word	
open				proto	fname:ptr byte, mode:word
read				proto	fd:word, buf:ptr byte, n:word
write				proto	fd:word, buf:ptr byte, n:word
close				proto	fd:word
ClearDosError	proto

puts				proto	s:ptr byte
sprintf			proto	s:ptr byte, fp:ptr byte, arg:vararg
vsprintf			proto	s:ptr byte, fp:ptr byte, argPtr:ptr byte

ifndef SCI
sci_fgets		proto	s:ptr byte, len:word, fd:word
endif


endif

