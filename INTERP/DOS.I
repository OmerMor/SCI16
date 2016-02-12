;-------------------------------------------------------------------
;	dos.i
;-------------------------------------------------------------------

ifndef	DOS_I

DOS_I		equ	1


;DOS function calls
dosResetDisk	equ	0dh
dosSetDrive	   equ	0eh
dosEquipCfg	   equ	11h
dosCurDrive	   equ	19h
dosSetDTA	   equ	1ah
dosSetVector	equ	25h
dosGetVector	equ	35h
dosFreeSpace	equ	36h
dosCreat	      equ	3ch
dosOpen	      equ	3dh
dosClose	      equ	3eh
dosRead	      equ	3fh
dosWrite	      equ	40h
dosUnlink      equ	41h
dosLSeek	      equ	42h
dosDup		   equ	45h
dosCurDir	   equ	47h
dosFirst	      equ	4eh
dosNext		   equ	4fh
dosRename		equ	56h
dosFileDT	   equ	57h


CleanDir				proto		dir:ptr byte
KCheckFreeSpace	proto		args:ptr word
KValidPath			proto		args:ptr word
KDeviceInfo			proto		args:ptr word
ExitThroughDebug	proto
GetDevice			proto		path:ptr byte, device:ptr byte
GetCurDevice		proto		device:ptr byte
GetExecDir			proto		s:ptr byte


endif

