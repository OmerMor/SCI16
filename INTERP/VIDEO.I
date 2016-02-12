;----------------------------------------------------------------
; video.i
;
; VIDEO device driver, common header
;----------------------------------------------------------------

ifndef	VIDEO_I

VIDEO_I	equ	1


ifndef	DRIVER_I
include	driver.i
endif

;; structures ===================================================

;	.DATA

Cursor	struc
	CURxDim		   dw 00010h         ;CURxDim
	CURyDim		   dw 00010h         ;CURyDim
	CURxHot		   dw 0000h          ;CURxhot  = (xDim-1)/2 - xHot
	CURyHot		   dw 0000h          ;CURyhot  = yDim -1 -yHot
	CURskipColor	db 023h           ;CURskipcolor (one that won't work)
	Unused		   db 000h           ;availble
Cursor	ends

;; macros =======================================================


pushall	macro
	pushf
	push	ax
	push	bx
	push	cx
	push	dx
	push	bp
	push	si
	push	di
	push	ds
	push	es
	endm

popall	macro
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bp
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	popf
	endm


;; equates ======================================================

videoInt	equ	10h

; virtual screen size
MINY		equ	0
MINX		equ	0

; standard driver functions
;D_DETECT	equ	0
;	Do functional presence test for Install program
;	inputs: NONE
;	result: Currently no return

;D_INIT	equ	2
;	inputs: NONE
;	result: AX = previous mode

;D_TERMINATE equ	4
;	inputs: AX = mode to return to
;	result: NONE

; Video driver extensions

V_SHOWBITS equ	6
; ShowBits
;	inputs: Clipped rect in AX,BX,CX,DX, segment in SI
;	result: NONE

V_SHOWCUR 	equ	8
; RShowCursor
;	inputs: NONE
;	result: AX = curVisLevel

V_HIDECUR	equ	10
; RHideCursor
;	inputs: NONE
;	result: AX = curVisLevel

V_MOVECUR	equ	12
; MoveCursor
;	inputs: New X/Y in AX, BX
;	result: NONE

V_SETCUR	equ	14
; RSetCursor
;	inputs: Cursor data at AX:BX
;	result: AX = curVisLevel

V_SHAKE	equ	16
; ShakeScreen
;	inputs: 	AX:BX is address of 60 cycle ticks variable
;		CX is count, DX = direction
;	result:  NONE

V_SHIFT	equ	18
; ShiftScreen
;   input:   AX = rect top																  */
;            BX = rect left														     */
;            CX = rect bottom											 				  */
;            DX = rect right												  			  */
;            SI = direction
;            DI = vSeg
;            BP = V_SHIFT
;   output:  NONE

V_SETPALETTE		equ	20
; RSetPalette
;  AX:SI is pointer to palette data

V_GETPALETTE		equ	22
; RSetPalette
;  AX:SI is pointer to palette data


V_SETPALETTE_CYCLE		equ	24
; RSetPalette
;  AX:SI is pointer to palette data

V_RECT_DISPLAY equ 26
V_SETMODE equ 28
V_DISPLAYPAGE equ 30
V_COPYPAGE equ 32
V_SETUPVMAP equ 34
V_HWSHIFT	equ	36
; ShiftScreen
;   input:   AX = direction
;            BP = V_HWSHIFT
;   output:  NONE


V_SETOUTPAGE	equ	38
V_GETOUTPAGE	equ	40
V_CLEARPAGEMODE equ	42
endif

