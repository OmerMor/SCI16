;-------------------------------------------------------------------
; KERNDISP.S
; Dispatch table for kernel routines.
; When adding a function, add it in two places:
;   extkern
;   kernel
;-------------------------------------------------------------------

include macro.i

include kerndisp.i

ifdef	DEBUG

.CONST
.DATA
.DATA?
.STACK

;; This segment will contain a table of pointers to the names of
;; the kernel function calls.
KNamePtrs	segment	public word 'DATA'
kernelNames	label	word
KNamePtrs	ends

;; This segment will contain the actual names of the functions, pointers
;; to which will be in the segment KNamePtrs.
KNames		segment	public byte 'DATA'
dummy_name	db		"Dummy", 0
KNames		ends

DGROUP	group	CONST,_BSS,_DATA,STACK,KNamePtrs

endif

;; This macro does all the work to add a kernel function call.  It declares
;; the function as external, puts its address in the dispatch table, stores
;; the name of the function, and puts a pointer to that name in the
;; kernelNames table.
kernel	macro	func
.CODE
extrn	K&func:proc
.DATA
	dword	K&func
ifdef	DEBUG
KNames	segment
func&_name	byte	"&func", 0
KNames	ends
KNamePtrs	segment
	word	offset DGROUP:func&_name
KNamePtrs	ends
endif
	endm


;; This macro creates dummy (placeholder) entries in the tables.
dummy		macro	func
.CODE
extrn	KDummy:proc
.DATA
	dword	KDummy
ifdef	DEBUG
KNamePtrs	segment
	word	offset DGROUP:dummy_name
KNamePtrs	ends
endif
	endm

.DATA

kernelDispTbl	label	dword

	;Resource handling.
	kernel	Load
	kernel	UnLoad
	kernel	ScriptID
	kernel	DisposeScript

	;Object management.
	kernel	Clone
	kernel	DisposeClone
	kernel	IsObject
	kernel	RespondsTo

	;Pictures.
	kernel	DrawPic
	kernel	Show
	kernel	PicNotValid							;10

	;Animated objects & views.
	kernel	Animate
	kernel	SetNowSeen
	kernel	NumLoops
	kernel	NumCels
	kernel	CelWide
	kernel	CelHigh
	kernel	DrawCel
	kernel	AddToPic

	;Window/dialog/controls.
	kernel	NewWindow
	kernel	GetPort								;20
	kernel	SetPort
	kernel	DisposeWindow
	kernel	DrawControl
	kernel	HiliteControl
	kernel	EditControl

	;Screen text.
	kernel	TextSize
	kernel	Display

	;Events.
	kernel	GetEvent
	kernel	GlobalToLocal
	kernel	LocalToGlobal						;30
	kernel	MapKeyToDir

	;Menu bar & status line.
	kernel	DrawMenuBar
	kernel	MenuSelect
	kernel	AddMenu
	kernel	DrawStatus

	;Parsing.
	dummy												;;;;;;;;;;;;;;;;;;; dummy
	dummy												;;;;;;;;;;;;;;;;;;; dummy
	dummy												;;;;;;;;;;;;;;;;;;; dummy

	;Mouse functions.
	kernel	HaveMouse
	kernel	SetCursor							;40


	;Save/restore/restart.
	kernel	SaveGame
	kernel	RestoreGame
	kernel	RestartGame
	kernel	GameIsRestarting

	;Sounds.
	kernel	DoSound

	;List handling.
	kernel	NewList
	kernel	DisposeList
	kernel	NewNode
	kernel	FirstNode
	kernel	LastNode								;50
	kernel	EmptyList
	kernel	NextNode
	kernel	PrevNode
	kernel	NodeValue
	kernel	AddAfter
	kernel	AddToFront
	kernel	AddToEnd
	kernel	FindKey
	kernel	DeleteKey

	;Mathematical functions.
	kernel	Random								;60
	kernel	Abs
	kernel	Sqrt
	kernel	GetAngle
	kernel	GetDistance

	;Miscellaneous.
	kernel	Wait
	kernel	GetTime

	;String handling.
	kernel	StrEnd
	kernel	StrCat
	kernel	StrCmp
	kernel	StrLen								;70
	kernel	StrCpy
	kernel	Format
	kernel	GetFarText
	kernel	ReadNumber

	;Actor motion support.
	kernel	BaseSetter
	kernel	DirLoop
	kernel	CantBeHere
	kernel	OnControl
	kernel	InitBresen
	kernel	DoBresen								;80
	kernel	Platform
	kernel	SetJump

	;Debugging support.
	kernel	SetDebug
	kernel	InspectObj
	kernel	ShowSends
	dummy												;;;;;;;;;;;;;;;;;;; dummy
	kernel	ShowFree
	kernel	MemoryInfo
	kernel	StackUsage
	kernel	Profiler								;90

	; new menu calls
	kernel	GetMenu
	kernel	SetMenu

	kernel	GetSaveFiles
	kernel	GetCWD
	kernel	CheckFreeSpace
	kernel	ValidPath

	kernel	CoordPri
	kernel	StrAt
	kernel	DeviceInfo
	kernel	GetSaveDir							;100
	kernel	CheckSaveGame
	
	kernel	ShakeScreen
	kernel	FlushResources

	; integer trigonometry calls - Pablo Ghenis
	;
	kernel	SinMult
	kernel	CosMult
	kernel	SinDiv
	kernel	CosDiv

	kernel	Graph
	kernel	Joystick
	kernel	ShiftScreen							;110
	kernel	Palette								

	kernel	MemorySegment
	kernel	PalVary

	kernel	Memory
	kernel	ListOps
	kernel	FileIO

	kernel	DoAudio
	kernel	DoSync

	kernel	AvoidPath
	kernel	Sort									;120
	kernel	ATan									
	kernel	Lock

	kernel   RemapColors

	kernel	Message
	kernel	IsItSkip
	kernel	MergePoly

	;Resource handling (...continued)
	kernel	ResCheck
	kernel	AssertPalette

	kernel	TextColors
	kernel	TextFonts							;130

	;Event macros
	kernel	Record
	kernel	PlayBack

	kernel	ShowMovie
	kernel	SetVideoMode

	kernel	SetQuitStr
	kernel	DbugStr
kernelMax	dw	($-kernelDispTbl)/4 - 1


	end
