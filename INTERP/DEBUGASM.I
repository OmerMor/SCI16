;-------------------------------------------------------------------
;	debugasm.i
;-------------------------------------------------------------------

ifndef DEBUGASM_I

DEBUGASM_I	equ	1

DebugInit		proto
RDebugger		proto near
KStackUsage		proto	funcPtr:word

DebugOn			proto
DebugOff			proto
StackUsage   	proto	func:word
ShowFreeList	proto	where:word

ProfileHandler	proto
DoTrace			proto

endif
