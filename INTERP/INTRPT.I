;-------------------------------------------------------------------
;	intrpt.i
;-------------------------------------------------------------------

ifndef INTRPT_I
INTRPT_I		equ	1

Server struc
	inUse       word		0  ;is server active?
	freq		   word		0	;desired freqency of service
	count		   word		0	;count of ticks to next service
	function		dword		0	;pointer to server function
Server ends

MaxServers     equ   4

externdef servers: Server
externdef sysTicks: dword
externdef diskIOCritical: word

SetInterrupts		proto
ResetInterrupts	proto
SetDisneyInt		proto
InstallServer		proto	procPtr:far ptr, ticks:word
DisposeServer		proto	procPtr:far ptr
InterruptsOn		proto
InterruptsOff		proto
RTickCount			proto
SysTime				proto	func:word
TimerIntHandler	proto
BreakExitHandler	proto
CritErrorHandler	proto

endif

