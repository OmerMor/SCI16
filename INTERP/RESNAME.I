;-------------------------------------------------------------------
;	resname.i
;-------------------------------------------------------------------

ifndef	RESNAME_I

RESNAME_I	equ	1

ResNameMake				proto	resType:word, resId:word
ResNameMakeWildCard	proto	resType:word
ResNameMatch			proto	theName:ptr byte
ResName					proto	resType:word
ROpenResFile			proto	resType:word, resId:word, theName:ptr byte
FindResInPath			proto	resType:word, resId:word, theName:ptr byte, len:ptr dword
addSlash					proto	dir:ptr byte


endif

