;-------------------------------------------------------------------
;	driver.i
;
;	Definitions for drivers.
;-------------------------------------------------------------------

DRIVER_I EQU 1


;Magic number to identify drivers.
DRIVER		equ	87654321h

;Driver types.
D_VIDEO		equ	0
D_MUSIC		equ	1
D_VOICE		equ	2
D_INPUT		equ	3
D_KEYBOARD	equ	4
D_COMM		equ	5
D_MOUSE		equ	6
D_MEMORY	equ	7	; type used for autodetection purposes only

;Required driver functions.
D_DETECT	equ	0
D_INIT		equ	2
D_TERMINATE	equ	4

;
EXTDRV		equ	0FEDCBA98H

; extended driver bits

; video
MDA_BIT		equ	1
HERCULES_BIT	equ	2
CGA_BIT		equ	4
IBMPCJR_BIT	equ	8
TANDY_BIT	equ	10H		;16
EGA_BIT		equ	20H		;32
MCGA_BIT	equ	40H		;64
VGA_BIT		equ	80H		;128
CGAM_BIT	equ	100H		;256
CGAC_BIT	equ	200H		;512
EXPLORER_BIT	equ	400H		;1024

;keyboard
IBMKBD_BIT	equ 1
TANDYKBD_BIT	equ 2
NECKBD_BIT	equ 4

;sound
STD_SPEAKER		EQU 0001h
ADLIB			EQU 0002h
SBLAST_DAC		EQU 0004h
CMS			EQU 0008h
TANDY_3VOICE	        EQU 0010h
TANDY_DAC		EQU 0020h
PS1_3VOICE		EQU 0040h
PS1_DAC			EQU 0080h
SBLAST_PRO		EQU 0100h
MPU_MIDI		EQU 0200h
DISNEY_DAC		EQU 0400h
CDAUDIO_OUT		EQU 0800h
PROAUDIO_FM		EQU 1000h
ARIA              EQU 10000h
GRAVIS			EQU 20000h
SADLIB			EQU 40000h
PROAUDIO_DAC	        EQU 2000h
WINDOWS_SS	        EQU 4000h
NO_MIDI 		EQU 8000h

