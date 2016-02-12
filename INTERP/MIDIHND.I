;---------------------------------------------------------------------------
; midihnd.i		resource handle/blockID dereferencing for MIDI.S
;					this version for SCI
;					Mark Wilden, April 1991
;---------------------------------------------------------------------------

;---------------------------------------------------------------------------
; Hnd2Ptr		converts an SCI resource handle to a pointer
;					resource header
;
; entry:	id		= resource handle
; exit:	ds:bp	= pointer
;---------------------------------------------------------------------------


ifndef	MIDIHND_I

MIDIHND_I	equ	1


Hnd2Ptr macro	id
	lds	bp, dword ptr id
	lds	bp, ds:[bp]
endm



endif

