;---------------------------------------------------------------------------
; armasm.i		Mark Wilden, May 1992
;
; interface to assembly helper routines for alternate resource memory access
;---------------------------------------------------------------------------

ifndef ARMASM_I
ARMASM_I equ	1

ifndef ALTRES_I
include altres.i
endif

; page parameters
PAGESIZE_LOG2		equ 14
PAGESIZE				equ 1 shl PAGESIZE_LOG2
MAXPAGES				equ 512									; allows for 512 * 16K = 8M
MAXPAGES_REQUEST	equ 1 shl (16 - PAGESIZE_LOG2)	; 64K / 16K
MINPAGES				equ MAXPAGES_REQUEST * 2

; direction selectors
PAGE2MEM		equ	0
MEM2PAGE		equ	1

ARMAlloc				proto arm:ptr ARMType, count:word
ARMCopy				proto direction:word, arm:ptr ARMType, aPage:word,
							aOffset: word, count:word, data:far ptr
ARMAllocPageMap	proto arm:ptr ARMType
ARMFree				proto arm:ptr ARMType, aPage:word
ARMGetFreePage		proto arm:ptr ARMType
ARMFreePageMap		proto arm:ptr ARMType
ARMCritical			proto


endif
