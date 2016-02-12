;-------------------------------------------------------------------
;	ems.i		EMS interface
;-------------------------------------------------------------------

ifndef EMS_I
EMS_I		equ	1

ifndef ALTRES_I
include altres.i
endif

externdef	ems:ARMType

EMSDetect	proto
EMSAlloc		proto nPages:word
EMSMapPage	proto physicalPage:byte, logicalPage:word, aHandle:word
EMSFree		proto aHandle:word

endif
