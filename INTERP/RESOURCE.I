;-------------------------------------------------------------------
;	resource.i
;-------------------------------------------------------------------

ifndef	RESOURCE_I

RESOURCE_I	equ	1

externdef resourceCursorType:word
externdef checkingLoadLinks:bool

FindResEntry	proto	resType:word, resID:word
GetResHandle	proto	memSize:word
PurgeLast		proto
LoadHandle		proto	fileName:ptr byte
ResLoad			proto	resType:word, resID:word
ResCheck		proto	resType:word, resID:word
ResLock			proto	resType:word, resID:word, lockIt:word
ResUnLoad		proto	resType:word, resID:word
FindPatchEntry	proto	resType:word, resID:word
InitPatches		proto

endif

