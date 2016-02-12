;-------------------------------------------------------------------
;	event.i
;	Definitions for input device drivers.
;-------------------------------------------------------------------

ifndef EVENT_I
EVENT_I	equ	1

ifndef	GRAPH_I
include	graph.i
endif

ifndef	OBJECT_I
include	object.i
endif

;-------------------------------------------------------------------
; equates
;-------------------------------------------------------------------

;Device function calls
INP_POLL			equ	6
INP_MODIFIERS	equ	8
INP_MAP			equ	10

JOY_RPT_INT	equ	12;Interval (SysTicks) between forced joystick reports
				;Value for interval is passed in SI
				;Special values are:
				;  0  only report when joystick changes
				; -1  return the current value

;Event types
nullEvt		equ	0000h
mouseDown	equ	0001h
mouseUp		equ	0002h
keyDown		equ	0004h
keyUp			equ	0008h
menuStart	equ	0010h
menuHit		equ	0020h
direction	equ	0040h
saidEvent	equ	0080h
joyDown		equ	0100h
joyUp			equ	0200h

allEvents	equ	7fffh

;Event modifiers.
shft 			equ	3  		;either shift key
ctrl 			equ	4  		;control key
alt  			equ	8  		;alt key

;Key definitions.
HOMEKEY		equ	04700h
UPARROW		equ	04800h
PAGEUP		equ	04900h
LEFTARROW	equ	04b00h
CENTERKEY	equ	04c00h
RIGHTARROW	equ	04d00h
ENDKEY		equ	04f00h
DOWNARROW	equ	05000h
PAGEDOWN		equ	05100h
DELETE		equ	05300h
PAUSEKEY		equ	07000h

SHFTUP			equ	038h
SHFTPAGEUP		equ	039h
SHFTRIGHT		equ	036h
SHFTPAGEDOWN 	equ	033h
SHFTDOWN			equ	032h
SHFTEND			equ	031h
SHFTLEFT			equ	034h
SHFTHOME			equ	037h
SHFTCENTER		equ	035h

TNDYSHFTUP		equ	08500h
TNDYSHFTRIGHT	equ	08800h
TNDYSHFTDOWN	equ	08600h
TNDYSHFTLEFT	equ	08700h

F1		equ	03b00h
F2		equ	03c00h
F3		equ	03d00h
F4		equ	03e00h
F5		equ	03f00h
F6		equ	04000h
F7		equ	04100h
F8		equ	04200h
F9		equ	04300h
F10	equ	04400h

;Direction messages.
dirStop	equ	0
dirN		equ	1
dirNE		equ	2
dirE		equ	3
dirSE		equ	4
dirS		equ	5
dirSW		equ	6
dirW		equ	7
dirNW		equ	8

;-------------------------------------------------------------------
; structures
;-------------------------------------------------------------------

Event	struc
	eType			word		?
	message		word		?
	modifiers	word		?		; this is shared by objID in SCI
	when			dword		?
	where			RPoint	<>
Event	ends

EventPtr	typedef	ptr Event

;-------------------------------------------------------------------
; externals
;-------------------------------------------------------------------
externdef	evHead:EventPtr
externdef	evTail:EventPtr
externdef	evQueue:EventPtr
externdef	evQueueEnd:EventPtr
externdef	kbdHandle:dword
externdef	keyboard:dword		;pointer to keyboard driver
externdef	joyHandle:dword
externdef	joystick:dword

InitEvent		proto		nEvents:word
EndEvent			proto
RGetNextEvent	proto		evtMask:word, event:ptr Event
RFlushEvents	proto		evtMask:word
REventAvail		proto		evtMask:word, event:ptr Event
RStillDown		proto
RPostEvent		proto		event:ptr Event
MakeNullEvent	proto		event:ptr Event
KMapKeyToDir	proto		key:ptr word
EventToObj		proto		event:ptr Event, obj:ptr Obj
ObjToEvent		proto		obj:ptr Obj, event:ptr Event
KJoystick		proto		w:ptr word
MapKeyToDir		proto		event:ptr Event

endif

