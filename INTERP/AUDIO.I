;-------------------------------------------------------------------
;	audio.i
;-------------------------------------------------------------------

ifndef	AUDIO_I

AUDIO_I EQU 1


;Audio resource ID
AUDIO			equ	8dh

;Codes to be sent to the audio driver (in BP).
A_STAT		equ	1	; check status
A_MEMPLAY	equ	3
A_MEMCHECK	equ	4
A_MEMSTOP	equ	5

externdef	audioDrv:word
externdef	cdaudioDrv:word
externdef	audNone:word


endif

