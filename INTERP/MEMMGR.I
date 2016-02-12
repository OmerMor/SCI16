;-------------------------------------------------------------------
;	memmgr.i
;-------------------------------------------------------------------

ifndef MEMMGR_I
MEMMGR_I		equ	1

ifndef LIST_I
include list.i
endif

;Free list structure for pointers
block	struc
	bSize		word	?	;size of pointer
	bNext		word	?	;pointer to next free block in list
block	ends

blockPtr	typedef ptr block

externdef 	freeList:blockPtr
externdef	hunkAvail:word
externdef	heapAvail:word
externdef	heapBase:word
externdef	handleBase:word

InitHeap				proto	theBase:word, theSize:word
InitHandle			proto	baseSeg:word, paraSize:word
InitMem				proto	maxHunkSize:word
RNewPtr				proto	theSize:word
NeedPtr				proto	theSize:word
FillPtr				proto	thePtr:word, theByte:byte
ClearPtr				proto	thePtr:word
DisposePtr			proto	thePtr:word
ReallocatePtr		proto	thePtr:word, newSize:word
LargestPtr			proto
FreeHeap				proto
PtrSize				proto	thePtr:word
SaveHeap				proto
RestartHeap			proto
memcpy				proto	dest:near ptr byte, source:near ptr byte, count:word
FarMemcpy			proto	dest:far ptr byte, source:far ptr byte, count: word
RelocateHunk		proto
PackHandles			proto
GetHandle			proto
RNewHandle			proto	lsize:word
NeedHandle			proto	lsize:word
FillHandle			proto	theHandle:word, theByte:word
ClearHandle			proto	theHandle:word
ReturnHandle		proto	theHandle:word
DisposeHandle		proto	theHandle:word
DisposeHunk			proto near
ReallocateHandle	proto	theHandle:word, newSize:word
LargestHandle		proto
FreeHunk				proto
HandleSize			proto	theHandle:word
HandleLocked		proto	theHandle:word
LockHandle			proto	theHandle:word
UnlockHandle		proto	theHandle:word
CriticalHandle		proto	theHandle:word, critical:word
hunkcpy				proto	dest:word, src:dword, len:word
CopyHandle			proto	theHandle:word, thePtr:word
StackLeft			proto
CheckLoadLinks		proto

endif

