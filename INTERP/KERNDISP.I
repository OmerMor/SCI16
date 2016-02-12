;-------------------------------------------------------------------
;	kerndisp.i
;-------------------------------------------------------------------
ifndef KERNDISP_I
KERNDISP_I	equ	1

externdef kernelDispTbl: dword
externdef kernelMax: word
externdef kernelNames: word

KernelCall	typedef proto far argPtr: word

endif

