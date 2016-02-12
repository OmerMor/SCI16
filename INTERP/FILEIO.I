;-------------------------------------------------------------------
;	fileio.i
;-------------------------------------------------------------------

ifndef	FILEIO_I

FILEIO_I	equ	1


externdef criticalError:word

readfar			proto		fd:word, buffer:word, len:word
ReadDos			proto		fd:word, buffer:far ptr byte, len:word
WriteDos			proto		fd:word, buffer:far ptr word, len:word
filelength		proto		fd:word
unlink			proto		filename:ptr byte
firstfile		proto		spec:ptr byte, atr:word, dta:ptr byte
nextfile			proto		dta:ptr byte
getcwd			proto		dir:ptr byte
existdrive		proto		drive:word
ResetDisk		proto
RGetFreeSpace	proto		drive:byte
Exists			proto		filename:ptr byte
drivecheck		proto		pathname:ptr byte
lseek				proto		fd:word, file_ofs:dword, theOrg:word


endif

