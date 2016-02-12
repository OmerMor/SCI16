;-------------------------------------------------------------------
;	restypes.i
;-------------------------------------------------------------------

ifndef	RESTYPES_I

RESTYPES_I	equ	1

RES_BASE		equ	080h
RES_VIEW		equ	080h
RES_PIC		equ	081h
RES_SCRIPT	equ	082h
RES_TEXT		equ	083h
RES_SOUND	equ	084h
RES_MEM		equ	085h
RES_VOCAB	equ	086h
RES_FONT		equ	087h
RES_CURSOR	equ	088h
RES_PATCH	equ	089h
RES_BITMAP	equ	08ah
RES_PAL		equ	08bh
RES_CDAUDIO	equ	08ch
RES_AUDIO	equ	08dh
RES_SYNC		equ	08eh
RES_MSG		equ	08fh
RES_MAP		equ	090h
RES_HEAP		equ	091h

;; These are here for compatability only -- the above should be used
;; when writing new code.

BASETYPE		equ	RES_BASE
VIEW			equ	RES_VIEW
PICTURE		equ	RES_PIC
SCRIPT		equ	RES_HUNK
TEXT			equ	RES_TEXT
SOUND			equ	RES_SOUND
MEMORY		equ	RES_MEM
VOCAB			equ	RES_VOCAB
FONT			equ	RES_FONT
CURSOR		equ	RES_CURSOR
PATCH			equ	RES_PATCH
BITMAP		equ	RES_BITMAP
PALETTE		equ	RES_PAL
CDAUDIO		equ	RES_CDAUDIO
AUDIO			equ	RES_AUDIO
SYNC			equ	RES_SYNC
MSG			equ	RES_MSG
MAP			equ	RES_MAP


endif

