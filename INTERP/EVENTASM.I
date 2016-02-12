;-------------------------------------------------------------------
;	eventasm.i
;-------------------------------------------------------------------

ifndef	EVENTASM_I

EVENTASM_I EQU 1

PollKeyboard	proto
FlushKeyboard	proto
KeyboardDriver	proto		func:word, args:vararg
GetModifiers	proto
PollJoystick	proto
JoystickDriver	proto		func:word, args:vararg


endif

