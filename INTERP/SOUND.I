ifndef SOUND_I
SOUND_I EQU 1

IFNDEF DRIVER_I
include	driver.i
ENDIF

SOUND		EQU	84h

Sound		STRUC
sNext		dw 0		;Node header
sPrev		dw 0		;Node header
sKey		dw 0		;Node header
sPointer	dd 0		;Far pointer to sound resource
tIndex		dw 16 dup (0)	;Index for 16 tracks
tLoopPoint	dw 16 dup (0)	;Loop RPoint for 16 tracks
tRest		dw 16 dup (0)	;Rest count for 16 tracks
tLoopRest	dw 16 dup (0)	;Rest count at loop for 16 tracks
tChannel	db 16 dup (0)	;Channel #'s for 16 tracks
tCommand	db 16 dup (0)	;Last command for 16 tracks (running status) 
tLoopCommand	db 16 dup (0)	;Command at loop for 16 tracks
cDamprPbend	dw 15 dup (0)	;Damper pedal and pitch bend for 15 channels
cPriVoice	db 15 dup (0)	;Priority & voice allocation for 15 channels
cModulation	db 15 dup (0)	;Modulation values for 15 channels
cPan		db 15 dup (0)	;Pan values for 15 channels
cVolume		db 15 dup (0)	;Volume values for 15 channels
cProgram	db 15 dup (0)	;Program values for 15 channels
sSignal		db 0		;Sound signal
sDataInc	dw 0		;Current Data Increment cue value
sTimer		dw 0		;Age of sound (in 60th secs)
sLoopTime	dw 0		;Timer loop point
sState		db 0		;Flags sound as a bed sound (used in restore)
sPriority	db 0		;Sound priority
sLoop		db 0		;Sound loops
sVolume		db 0		;Sound volume
sFadeDest	db 0		;Fade destination
sFadeTicks	db 0		;Clock ticks before next fade
sFadeCount	db 0		;Fade tick counter
sFadeSteps	db 0		;Fade steps
sPause		db 0		;Pause flag
Sound		ENDS

;--------------------------------------------------------------------------
;--				Old Stuff
;--------------------------------------------------------------------------

;System sound cue
SND_DONE            equ  -1
SND_START           equ   0

;Sound states
SND_NOTREADY        equ  0
SND_READY           equ  1
SND_BLOCKED         equ  2
SND_ACTIVE          equ  3

externdef	soundList:word
externdef	reverbDefault:word

ENDIF
