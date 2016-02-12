;-------------------------------------------------------------------
;	start.i
;-------------------------------------------------------------------

ifndef START_I

START_I	equ	1

externdef argc: word
externdef argv: ptr word
externdef bssStart: byte
externdef bssEnd: byte
externdef numHandles: word
externdef pspSeg: word
externdef stackBase: word
externdef panicStr: word
externdef quitStr: byte
externdef commandLine: byte

exit						proto exitCode: byte
ExitFromC				proto	exitCode: byte
MakeCommandLineArgs	proto	off: word, segm: word
onexit					proto func: FuncPtr
Start						proto
WriteChar				proto chr: byte

endif
