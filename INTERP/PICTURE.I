;-------------------------------------------------------------------
;	picture.i
;-------------------------------------------------------------------

ifndef PICTURE_I
PICTURE_I	equ	1

externdef	picNotValid:word

PriBands		proto	b:far ptr byte
InitPri		proto	w1:word, w2:word
InitPicture	proto
ShowPic		proto	w1:word, w2:word
CoordPri		proto	w:word
PriCoord		proto	w:word

; mirroring stuff
VMIRROR		equ	2000h
HMIRROR		equ	4000h

endif

