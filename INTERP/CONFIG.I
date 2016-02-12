;-------------------------------------------------------------------
;	config.i
;-------------------------------------------------------------------

ifndef	CONFIG_I

CONFIG_I		equ	1

externdef videoDriver:word
externdef soundDriver:word
externdef kbdDriver:word
externdef joyDriver:word
externdef audioDriver:word
externdef patchDir:word
externdef useMouse:word
externdef useDiskAudio:word
externdef audioPort:word

externdef minHunkSize:word
externdef maxHunkUsed:word


endif

