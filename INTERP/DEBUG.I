;-------------------------------------------------------------------
;	debug.i
;-------------------------------------------------------------------

ifndef	DEBUG_I

DEBUG_I	equ	1

externdef	newRoomNum:word
externdef	traceOn:word
externdef	trace:word
externdef	lastSp:word
externdef   isDebugVersion:word

externdef	proOn:word
externdef	proBuffer:word
externdef	proIndex:word
externdef	proBufSize:word


KShowFree		proto
KSetDebug		proto		args:ptr word
KShowSends		proto		args:ptr word
KInspectObj		proto		args:ptr word

ShowSends		proto		w:word
SetDebug			proto		on:word
WriteResUse		proto		resType:word, resID:word
CheckHunkUse	proto		paragLocked:word
Debug				proto		sciIP:ptr byte, sciSP:ptr word, args:vararg
ToggleDebug		proto
Resources		proto
SizedWindow		proto		s:ptr byte, titleStr:ptr byte, w:word
DebugWindow		proto		s:ptr byte, titleStr:ptr byte, s:word, x:word, y:word, w:word
PError			proto		sciIP:ptr byte, sciSP:ptr byte, errCode:word, args:vararg


endif

