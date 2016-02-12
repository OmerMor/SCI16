;-------------------------------------------------------------------
;	palette.i
;-------------------------------------------------------------------

ifndef PALETTE_I
PALETTE_I EQU 1

danPalette struc 
	paletteHdSize	db	?
	sysPalName	   db	9 dup (?)
	paletteCount	db	?
	dummy		      dw	?
	paletteSize	   dw	?
	danPaltitle		db	9 dup (?)
	sysPalNum	   db	?
	startOffset	   db	?
	nCycles		   db	?
	dummy2		   dw	?
	nColors		   dw	?
	defaultFlag	   db	?
	theType		   db	?
	validStamp	   dd	?
danPalette ends

InitPalette			proto
KPalette				proto	args:ptr word
SetPalIntensity	proto	wp:far ptr word, w1:word, w2:word, w3:word
PalMatch				proto	r:word, g:word, b:word

SetResPalette		proto
RSetPalette			proto	pal:far ptr word, mode:word
RSetDanPalette		proto	pal:word
RemapByPercent		proto	

externdef			sysPalette:word
externdef			rempaPercent:word
externdef			palStamp:word

endif
