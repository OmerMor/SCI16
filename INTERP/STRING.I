;-------------------------------------------------------------------
;	string.i
;-------------------------------------------------------------------

ifndef	STRING_I

STRING_I		equ	1

strlen		proto	s:ptr byte
strcpy		proto	dest:ptr byte, src:ptr byte
strncpy		proto	dest:ptr byte, src:ptr byte, n:word
strdup		proto	s:ptr byte
strcat		proto	dest:ptr byte, src:ptr byte
strncat		proto	dest:ptr byte, src:ptr byte, n:word
strcmp		proto	s1:ptr byte, s2:ptr byte
strncmp		proto	s1:ptr byte, s2:ptr byte, n:word
strlwr		proto	s:ptr byte
strchr		proto	s:ptr byte, chr:word
strstr		proto	s1:ptr byte, s2:ptr byte
	
memcmp		proto	buf1:ptr byte, buf2:ptr byte, count:word
memcpy		proto	to:ptr byte, from:ptr byte, count:word

strtrn		proto	s1:ptr byte, s2:ptr byte, s3:ptr byte, s4:ptr byte
strbcat		proto	dest:ptr byte, src:ptr byte, count:word
reverse		proto	s:ptr byte
IsPrintStr	proto	s:ptr byte


endif

