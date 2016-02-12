;-------------------------------------------------------------------
;	object.i
;-------------------------------------------------------------------

ifndef	OBJECT_I

OBJECT_I		equ	1

Obj struc
	oObjID			word		?		;object identifier (should be OBJID)
	oSize				word		?		;number of properties in object
	oPropDict		word		?		;offset of property dictionary in hunk resource
	oMethDict		word		?		;offset of method dictionary in hunk resource
	oClassScript	word		?		;pointer to script node of object's class
	oScript			word		?		;pointer to script node for object
	oSuper			word		?		;pointer to super class
	oInfo				word		?		;bit-mapped information
Obj ends

ObjPtr	typedef	ptr Obj

ifdef	DEBUG
externdef	sendStack:word
externdef	ssPtr:word
externdef	ssEnd:word
endif

ClassEntry struct
	obj			word ?
	scriptNum	word	?
ClassEntry ends

ClassEntryPtr	typedef ptr ClassEntry

externdef classTbl: ClassEntryPtr

GetProperty		proto	obj:ptr Obj, property:word
SetProperty		proto	obj:ptr Obj, property:word, value:word
GetPropAddr		proto	obj:ptr Obj, property:word
QuickMessage 	proto
Messager			proto	
InvokeMethod	proto	obj:ptr Obj, selector:word, n:word, args:word
Clone				proto	theClass:ptr Obj
DisposeClone	proto	obj:ptr Obj
RespondsTo		proto	obj:ptr Obj, theSelector:word
IsObject			proto	obj:ptr Obj
HasPrintName	proto near
LoadClassTbl	proto

;---------------------------------------------------------------------------
; GetPropAddrMacro	return the address of a property for an object
;
; entry:	bx	= object
;			ax = property
; uses:	cx, dx
; exit:	bx	= address of property, or 0
;---------------------------------------------------------------------------
GetPropAddrMacro	macro
	local	NoProp,FoundProp

	push	di
	push  es

	;Check for valid object
	cmp   bx, MINOBJECTADDR
	jb    NoProp						;not an object--too low in memory
	test  bx, 1
	jne   NoProp						;not an object--odd address

	;get address of script node for this object's class
	mov	di, (Obj ptr [bx]).oClassScript
	;get handle of hunk resource from script node
	mov   di, (scriptNode ptr [di]).sHunk
	;get segment of hunk resource from handle
	mov   cx, (hunkPtr ptr [di]).hSegment
	mov   es, cx
	;get offset within hunk resource of property dictionary
	mov   di, (Obj ptr [bx]).oPropDict
	;get number of properties
	mov	cx, (Obj ptr [bx]).oSize
	mov	dx, cx
	repne	scasw
	jne	NoProp

	;Convert cx to word index into properties.
	inc   cx
	sub   dx,cx
	shl   dx,1
	add   bx,dx								;point bx at the property slot
	jmp	short FoundProp
NoProp:
	xor	bx, bx
FoundProp:
	pop   es
	pop	di
endm

endif

