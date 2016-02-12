;-------------------------------------------------------------------
; SAVEVARS.S
; Variables which need saving for a saved game.
;-------------------------------------------------------------------

include macro.i

include animate.i
include event.i
include graph.i
include memmgr.i
include menu.i
include mouse.i
include palette.i
include pmachine.i
include savevars.i
include script.i
include sound.i
include window.i

	.DATA?

saveStart	label word

;---event
evHead		         EventPtr	?
evTail		         EventPtr	?
evQueue		         EventPtr	?
evQueueEnd	         EventPtr	?

;---script
scriptList	         List	<>

;---window
wmgrPort 	         RGrafPortPtr	?
windowList	         List	<>		; List of currently open windows

;---memmgr
freeList		         blockPtr	?

;---menu
theMenuBar	         word	?

;---pmachine
ip			            memptr	?
pmsp			         wordPtr	?
acc			         word		?
pStack		         wordPtr	?
pStackEnd	         wordPtr	?
theVars		         label wordPtr
globalVar	         wordPtr	?
localVar	            wordPtr	?
tempVar		         wordPtr	?
parmVar		         wordPtr	?
restArgs             word		?
object		         ObjPtr	?
scriptHandle         Handle	?
classTbl	            ClassEntryPtr	?
numClasses	         word		?
defaultES            word		0
thisScript           word		0
thisIP               dword 	0

;---animate
lastCast	            word	?

;---sound
soundList	         List <>
reverbDefault			word	0

;---graph
palVaryOn        		db		0
palVaryPause     		db		0
palettePercent			dw		0
paletteDir				dw		0
paletteTime				dw		0
paletteStop				dw		0
paletteRes				dw		0
startPalette			dw		0
targetPalette			dw		0
newPalette				dw		0
palServerSignal		dw		0
currentVideoMode		byte	?

;---mouse
restrictRecPtr			RRectPtr	0
restrict					RRect <>

saveEnd	            LABEL WORD

	end
