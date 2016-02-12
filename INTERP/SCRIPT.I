;-------------------------------------------------------------------
;	script.i
;	Structures and external definitions for scripts.
;-------------------------------------------------------------------

ifndef SCRIPT_I
SCRIPT_I	equ	1

ifndef LIST_I
include list.i
endif

heapRes	struc
	hpFixOfs		word		?		;offset to fixup table
	hpNumVars	word		?		;number of variables
	hpVars		word		?		;start of variables
heapRes	ends

hunkRes	struc
	hkFixOfs		word		?		;offset to fixup table
	hkScript		word		?		;pointer to script node in heap
	hkFarText	word		?		;does this resource have far text?
	hkNumDisp	word		?		;number of dispatch table entries
	hkDispTbl	word		?		;beginning of dispatch table
hunkRes	ends

scriptNode	struc
	sNext	   	word		?		;list header on structure
	sPrev	   	word		?
	sKey 	   	word		?		;number of loaded script
	sHeap	   	word		?		;pointer to script in heap
	sHunk	   	word		?		;handle to script in hunk
	sVars			word		?		;pointer to variables for script
	sClones  	word		?		;number of clones spawned
scriptNode	ends

externdef 	scriptList:List

ScriptPtr		proto		n:word
LoadScript		proto		n:word

endif

