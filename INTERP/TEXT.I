;-------------------------------------------------------------------
;	text.i
;-------------------------------------------------------------------

ifndef	TEXT_I

TEXT_I	equ	1

CTRL_CHAR	equ	'|'

extrn textColors:ptr byte
extrn textFonts:ptr word
extrn	lastTextColor:byte
extrn	lastTextFont:byte

endif
