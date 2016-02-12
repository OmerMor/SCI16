;-------------------------------------------------------------------
;	altres.i		structures and defines for alternate memory types
;					duplicated in altres.h
;-------------------------------------------------------------------

ifndef ALTRES_I
ALTRES_I		equ 1

NARMTYPES	equ 5

ARMType	struc
	aName			word	0
	debugPrefix	byte	0
	init			dword	0
	memcpy		dword 0
	term			dword 0

	pageSize		word	0
	active		word	0

; the pageMap is handle to an array of links (much like a DOS FAT)
; if an entry is FREE_PAGE, the page is free
; if an entry is the same as the byte offset, this is the last page in the chain
; otherwise, the entry is the byte offset to the next page of the allocation
; the WORD offset into this array is the EMS logical page
	pageMap		word	0
	nPages		word	0		; number of total pages
	nFreePages	word 	0		; number of our free pages

ARMType	ends

AltResMemInit				proto
AltResMemAlloc				proto aSize: word, typ: ptr ptr ARMType
AltResMemRead				proto	loadLink:far ptr ptr
AltResMemWrite				proto	loadLink:far ptr ptr
AltResMemFree				proto	loadLink:far ptr ptr
AltResMemDebugPrefix		proto	loadLink:far ptr ptr
AltResMemDebugSummary	proto	where:ptr byte
AltResMemDebugKey			proto	where:ptr byte
AltResMemTerm				proto

NO_MEMORY	equ -1

endif
