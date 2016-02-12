; Project: Sierra Broadcasting Network
;
; Module: mserial.i
;
; Author: 	Jeff Stephenson
;		Matthew George
;
; Definitions for PC serial communications.




initreg	macro
lastReg = s_buffer
	endm


;Macro to send a value to a given register based on a port (which is assumed
;to be in the dx).  The 'initreg' macro must come before the first use of
;this macro in a procedure.  This macro should not be used inside conditionals
;unless both branches use it identically.  It should not be used inside loops.

outr	macro		reg, value
ifnb	<value>
ifdif	<value>,<al>
	mov	al, value
endif
endif
ife	reg-lastReg-1
	inc	dx
elseife	reg-lastReg+1
	dec	dx
elseif	reg-lastReg
	add	dx, reg-lastReg
endif
lastReg = reg
	out	dx, al
	endm


;Macro to get a value from a given register based on a port (which is assumed
;to be in the dx).  The 'initreg' macro must come before the first use of
;this macro in a procedure.  This macro should not be used inside conditionals
;unless both branches use it identically.  It should not be used inside loops.

inr		macro		reg
ife	reg-lastReg-1
	inc	dx
elseife	reg-lastReg+1
	dec	dx
elseif	reg-lastReg
	add	dx, reg-lastReg
endif
lastReg = reg
	in		al, dx
	endm




;Bits in the line control register

l_dlab				equ	80h			;divisor latch access bit

l_noParity			equ	00h
l_oddParity			equ	08h
l_evenParity		equ	0ch

l_1Stop				equ	00h
l_2Stop				equ	04h

l_7Bits				equ	02h
l_8Bits				equ	03h


;Bits in the line status byte.

l_timeOut			equ	80h
l_shiftEmpty		equ	40h
l_holdEmpty			equ	20h
l_break				equ	10h
l_framing			equ	08h
l_parity				equ	04h
l_overrun			equ	02h
l_dataReady			equ	01h


;Definitions for bits in the modem status byte.

m_detect				equ	80h
m_ring				equ	40h
m_dsr					equ	20h
m_cts					equ	10h
m_deltaDetect		equ	08h
m_trailEdge			equ	04h
m_deltaDSR			equ	02h
m_deltaCTS			equ	01h


;Definitions of bits in modem control register.

m_dtr					equ	01h
m_rts					equ	02h
m_intEnable			equ	08h
m_loop				equ	10h


;Definitions of the interrupt enable bits.

i_dataIntEnable	equ	01h
i_txIntEnable		equ	02h
i_lineIntEnable	equ	04h
i_modemIntEnable	equ	08h
i_allIntsEnable	equ	0fh


;Definitions of interrupt ID bits

i_noInt				equ	01h
i_lineInt			equ	06h
i_dataInt			equ	04h
i_txInt				equ	02h
i_modemInt			equ	00h


;Definitions of offsets of serial registers.

s_buffer				equ	0
s_divisorLow		equ	0
s_divisorHigh		equ	1
s_intEnable			equ	1
s_intID				equ	2
s_lineCtrl			equ	3
s_modemCtrl			equ	4
s_lineStatus		equ	5
s_modemStatus		equ	6

