;*************************************************************************
;*
;* MIDI Parser
;*
;* 07/13/92 - @DAC - Removed DAC code - JKT
;*
;* 08/04/92 - @INT - Interrupt added for MIDI receive - JKT
;*            @NUM - Enforced NUMNOTES cntl value to 0-15 - JKT  
;*
;*************************************************************************

		.MODEL	MEDIUM,C

MIDI_S		EQU		1

		include	midi.i
ifndef RECMID
                include	errmsg.i
endif


		.CODE

		PUBLIC	DoSound		; C Functions
                                               
		PUBLIC	PatchReq	;
		PUBLIC	Init		;
		PUBLIC	Terminate	;
		PUBLIC	ProcessSounds	;
		PUBLIC	SoundOn		;
		PUBLIC	RestoreSound	;
		PUBLIC	MasterVol	;
		PUBLIC	SetReverb	;
		PUBLIC	PlaySound	; 
		PUBLIC	EndSound	; Assembly functions
		PUBLIC	PauseSound	;
		PUBLIC	FadeSound	;
		PUBLIC	HoldSound	;
		PUBLIC	MuteSound	;
		PUBLIC	ChangeVol	;
		PUBLIC	ChangePri	;
		PUBLIC	GetSignal	;
		PUBLIC	GetDataInc	;
		PUBLIC	GetSMPTE	;
		PUBLIC	SendNoteOff	;
		PUBLIC	SendNoteOn	;
		PUBLIC	SendPChange	;
		PUBLIC	SendContrlr	;
		PUBLIC	SendPBend	;
		PUBLIC	SoundServer	;
		PUBLIC	GetSignalRset	;

		PUBLIC	PlayList	;
		PUBLIC	ChList		; Public tables
		PUBLIC	SampleList	;

ifdef RECMID
		PUBLIC	AskDriver       

                PUBLIC  StashMIDI
                 
		EXTRN	UpdateLists:FAR
endif

Cparam		EQU		2						;Stack offset for C params
LISTSIZE	EQU		16						;PlayList size (3 - 16)

;--------------------------------------------------------------------------
;				Tables

PlayList	dd 16 dup (0)
SampleList	dd 16 dup (0)
ChNodes		dd 16 dup (0)

EndSnds		dw 32 dup (0)
KludgeHdr	dw 16 dup (0)

Holding         db  0
ChList		db 16 dup (255)
ChBed		db 16 dup (0)
ChPri		db 16 dup (0)
ChVoice		db 16 dup (0)
ChNew		db 16 dup (255)
ChBedCopy	db 16 dup (0)
ChPriCopy	db 16 dup (0)
ChVoiceCopy	db 16 dup (0)
ChNewCopy	db 16 dup (255)
ChOld		db 16 dup (15)
VolRequest	db 16 dup (255)
savMuteFlag	db 15 dup (0)

ifdef RECMID	 
	
MIDIbuffer	db 140 dup (255)

;Variables for MIDI receive interrupt - @INT

MIDIBSIZE       EQU 4096                        ;MIDI interrupt buffer size
MIDIbufbeg      db MIDIBSIZE dup (0)            ;MIDI interrupt buffer
MIDIbufend      EQU MIDIbufbeg+MIDIBSIZE-1      ;MIDI buffer end address
MIDIbcount      dw 0                            ;MIDI interrupt buffer byte count
MIDIbhead       dw 0                            ;MIDI buffer empty pointer
MIDIbtail       dw 0                            ;MIDI buffer fill pointer

imask           db      0                       ;Interrupt mask
SavIntPtr       dw      0                       ;Saved interrupt vector (offset)
SavIntPtrS      dw      0                       ;Saved interrupt vector (segment)
install         db      0                       ;Install flag

il_base         EQU 8                           ;int number for IRQ0
midi_int        EQU 2                           ;Interrupt 2 
midi_level      EQU il_base+midi_int            ;midi interrupt level IRQ2
piccmd          EQU 20h                         ;8259 Priority int controller
picmsk          EQU 21h                         ;8259 Priority int controller

RunningStat     db      0    

endif

;---------------------------------------------------------------------------
;			     Variables

Driver		dd 0	      	;Far address of sound driver

SaveDS          dw 0

Carg1		dw 0	      	;
Carg2		dw 0	      	;
Carg3		dw 0	      	;C arguments to be passed
Carg4		dw 0	      	;
Carg5		dw 0	      	;
Carg6		dw 0	      	;
baseIndex	dw 0	      	;Base Index value for ParseNode PROC

processSnds	db 0	      	;ON\OFF switch for SoundServer
loChnl		db 0	      	;Lowest channel supported by driver
hiChnl		db 15	      	;Highest channel supported by driver
devID		db 0	      	;ID of the current music device
realChnl	db 255	        ;The channel# data is being sent on
ghostChnl	db 0		;Current channel is a ghost channel
numVoices	db 0		;Number of voices supported by driver
playPos		db 0		;Position of node on PlayList for ParseNode PROC
revDefault	db 0		;Default reverb mode
vLeftCopy	db 0		;Copy of voicesLeft
updateChnls	db 0		;Flag telling next interrupt to call DoChannelList
vRequest	db 0		;DoChangeVol flag
requestChnl	db 0		;Current VolRequest channel
oldReqChnl	db 0		;requestChnl end reference
oldRecCom	db 0		;Receiving mode running status
restoring	db 0		;Tells ParseNode not to send anything
testNode	db 0		;Flag signalling ParseNode to look at MIDI port
UARTmode	db 0		;Flag showing MPU in UART mode
priInfo		db 0

ifdef RECMID
.286
endif
CallDriver	MACRO	arg1
		push	bp
		mov	bp,arg1
		call	Driver
		pop	bp
		ENDM

getb			MACRO	arg1
		mov	arg1,ds:[bp]
		inc	bp
		shl	si,1
		inc	word ptr (Sound ptr es:[bx+si]).tIndex
		shr	si,1
		ENDM

BackupList	MACRO								;Backup ChNew, ChPri,
		push	ax									;ChVoice, ChBed, and
		mov	ax,word ptr ChNew				;voices left
		mov	word ptr ChNewCopy,ax
		mov	ax,word ptr ChNew[2]
		mov	word ptr ChNewCopy[2],ax
		mov	ax,word ptr ChNew[4]
		mov	word ptr ChNewCopy[4],ax
		mov	ax,word ptr ChNew[6]
		mov	word ptr ChNewCopy[6],ax
		mov	ax,word ptr ChNew[8]
		mov	word ptr ChNewCopy[8],ax
		mov	ax,word ptr ChNew[10]
		mov	word ptr ChNewCopy[10],ax
		mov	ax,word ptr ChNew[12]
		mov	word ptr ChNewCopy[12],ax
		mov	ax,word ptr ChNew[14]
		mov	word ptr ChNewCopy[14],ax
		mov	ax,word ptr ChPri
		mov	word ptr ChPriCopy,ax
		mov	ax,word ptr ChPri[2]
		mov	word ptr ChPriCopy[2],ax
		mov	ax,word ptr ChPri[4]
		mov	word ptr ChPriCopy[4],ax
		mov	ax,word ptr ChPri[6]
		mov	word ptr ChPriCopy[6],ax
		mov	ax,word ptr ChPri[8]
		mov	word ptr ChPriCopy[8],ax
		mov	ax,word ptr ChPri[10]
		mov	word ptr ChPriCopy[10],ax
		mov	ax,word ptr ChPri[12]
		mov	word ptr ChPriCopy[12],ax
		mov	ax,word ptr ChPri[14]
		mov	word ptr ChPriCopy[14],ax
		mov	ax,word ptr ChVoice
		mov	word ptr ChVoiceCopy,ax
		mov	ax,word ptr ChVoice[2]
		mov	word ptr ChVoiceCopy[2],ax
		mov	ax,word ptr ChVoice[4]
		mov	word ptr ChVoiceCopy[4],ax
		mov	ax,word ptr ChVoice[6]
		mov	word ptr ChVoiceCopy[6],ax
		mov	ax,word ptr ChVoice[8]
		mov	word ptr ChVoiceCopy[8],ax
		mov	ax,word ptr ChVoice[10]
		mov	word ptr ChVoiceCopy[10],ax
		mov	ax,word ptr ChVoice[12]
		mov	word ptr ChVoiceCopy[12],ax
		mov	ax,word ptr ChVoice[14]
		mov	word ptr ChVoiceCopy[14],ax
		mov	ax,word ptr ChBed
		mov	word ptr ChBedCopy,ax
		mov	ax,word ptr ChBed[2]
		mov	word ptr ChBedCopy[2],ax
		mov	ax,word ptr ChBed[4]
		mov	word ptr ChBedCopy[4],ax
		mov	ax,word ptr ChBed[6]
		mov	word ptr ChBedCopy[6],ax
		mov	ax,word ptr ChBed[8]
		mov	word ptr ChBedCopy[8],ax
		mov	ax,word ptr ChBed[10]
		mov	word ptr ChBedCopy[10],ax
		mov	ax,word ptr ChBed[12]
		mov	word ptr ChBedCopy[12],ax
		mov	ax,word ptr ChBed[14]
		mov	word ptr ChBedCopy[14],ax
		pop	ax
		mov	vLeftCopy,al
		ENDM

RestoreList	MACRO								;Restore ChNew, ChPri,
		push	ax									;ChVoice, ChBed, and
		mov	ax,word ptr ChNewCopy		;voices left
		mov	word ptr ChNew,ax
		mov	ax,word ptr ChNewCopy[2]
		mov	word ptr ChNew[2],ax
		mov	ax,word ptr ChNewCopy[4]
		mov	word ptr ChNew[4],ax
		mov	ax,word ptr ChNewCopy[6]
		mov	word ptr ChNew[6],ax
		mov	ax,word ptr ChNewCopy[8]
		mov	word ptr ChNew[8],ax
		mov	ax,word ptr ChNewCopy[10]
		mov	word ptr ChNew[10],ax
		mov	ax,word ptr ChNewCopy[12]
		mov	word ptr ChNew[12],ax
		mov	ax,word ptr ChNewCopy[14]
		mov	word ptr ChNew[14],ax
		mov	ax,word ptr ChPriCopy
		mov	word ptr ChPri,ax
		mov	ax,word ptr ChPriCopy[2]
		mov	word ptr ChPri[2],ax
		mov	ax,word ptr ChPriCopy[4]
		mov	word ptr ChPri[4],ax
		mov	ax,word ptr ChPriCopy[6]
		mov	word ptr ChPri[6],ax
		mov	ax,word ptr ChPriCopy[8]
		mov	word ptr ChPri[8],ax
		mov	ax,word ptr ChPriCopy[10]
		mov	word ptr ChPri[10],ax
		mov	ax,word ptr ChPriCopy[12]
		mov	word ptr ChPri[12],ax
		mov	ax,word ptr ChPriCopy[14]
		mov	word ptr ChPri[14],ax
		mov	ax,word ptr ChVoiceCopy
		mov	word ptr ChVoice,ax
		mov	ax,word ptr ChVoiceCopy[2]
		mov	word ptr ChVoice[2],ax
		mov	ax,word ptr ChVoiceCopy[4]
		mov	word ptr ChVoice[4],ax
		mov	ax,word ptr ChVoiceCopy[6]
		mov	word ptr ChVoice[6],ax
		mov	ax,word ptr ChVoiceCopy[8]
		mov	word ptr ChVoice[8],ax
		mov	ax,word ptr ChVoiceCopy[10]
		mov	word ptr ChVoice[10],ax
		mov	ax,word ptr ChVoiceCopy[12]
		mov	word ptr ChVoice[12],ax
		mov	ax,word ptr ChVoiceCopy[14]
		mov	word ptr ChVoice[14],ax
		mov	ax,word ptr ChBedCopy
		mov	word ptr ChBed,ax
		mov	ax,word ptr ChBedCopy[2]
		mov	word ptr ChBed[2],ax
		mov	ax,word ptr ChBedCopy[4]
		mov	word ptr ChBed[4],ax
		mov	ax,word ptr ChBedCopy[6]
		mov	word ptr ChBed[6],ax
		mov	ax,word ptr ChBedCopy[8]
		mov	word ptr ChBed[8],ax
		mov	ax,word ptr ChBedCopy[10]
		mov	word ptr ChBed[10],ax
		mov	ax,word ptr ChBedCopy[12]
		mov	word ptr ChBed[12],ax
		mov	ax,word ptr ChBedCopy[14]
		mov	word ptr ChBed[14],ax
		pop	ax
		mov	al,vLeftCopy
		ENDM

PreemptChn1	MACRO								;Preempt a channel off of
		push	di									;the ChNew list
		push	cx
		push	ax
		mov	dh,0ffh
		xor	ax,ax
		xor	di,di
p11:	cmp	al,ChPri[di]
		jae	p12
		mov	al,ChPri[di]
		mov	cx,di
		mov	dh,cl
p12:	inc	di
		cmp	di,16
		jne	p11
		pop	ax
		cmp	dh,0ffh
		je		p13
		xor	cx,cx
		mov	cl,dh
		mov	di,cx
		add	al,ChVoice[di]
		mov	ChNew[di],0ffh
		mov	ChVoice[di],0
		mov	ChPri[di],0
		mov	ChBed[di],0
p13:	pop	cx
		pop	di
		ENDM

PreemptChn2	MACRO								;Preempt a channel off of
		push	di									;the ChNew list
		push	cx
		push	ax
		mov	dh,0ffh
		xor	ax,ax
		xor	di,di
p21:	cmp	al,ChPri[di]
		jae	p22
		mov	al,ChPri[di]
		mov	cx,di
		mov	dh,cl
p22:	inc	di
		cmp	di,16
		jne	p21
		pop	ax
		cmp	dh,0ffh
		je		p23
		xor	cx,cx
		mov	cl,dh
		mov	di,cx
		add	al,ChVoice[di]
		mov	ChNew[di],0ffh
		mov	ChVoice[di],0
		mov	ChPri[di],0
		mov	ChBed[di],0
p23:	pop	cx
		pop	di
		ENDM

SwapChnls	MACRO								;Swap two channels on the
		push	ax									;ChNew list
		mov	al,ChNew[di]
		mov	ah,ChNew[si]
		mov	ChNew[di],ah
		mov	ChNew[si],al
		mov	al,ChPri[di]
		mov	ah,ChPri[si]
		mov	ChPri[di],ah
		mov	ChPri[si],al
		mov	al,ChVoice[di]
		mov	ah,ChVoice[si]
		mov	ChVoice[di],ah
		mov	ChVoice[si],al
		mov	al,ChBed[di]
		mov	ah,ChBed[si]
		mov	ChBed[di],ah
		mov	ChBed[si],al
		pop	ax
		ENDM

UpdateChannel1	MACRO		     ;Update a channel from node
		push	ax									;information
		push	dx
		push	si

		xor	ah,ah
		xchg	si,ax

		mov	cx,(ALLNOFF*256)
		CallDriver DController
		mov	cl,es:[bx+si].cPriVoice
		and	cl,0fh
		mov	ch,NUMNOTES
		CallDriver DController
		mov	cl,es:[bx+si].cProgram
		CallDriver DProgramChange
		push	si
		mov	si,ax
		mov	VolRequest[si],0ffh
		pop	si
		mov	cl,es:[bx+si].cVolume
		mov	dl,es:[bx].sVolume
		call	ScaleVolume
		mov	ch,VOLCTRL
		CallDriver DController
		mov	ch,PANCTRL
		mov	cl,es:[bx+si].cPan
		CallDriver DController
		mov	ch,MODCTRL
		mov	cl,es:[bx+si].cModulation
		CallDriver DController
		shl	si,1
		mov	cx,(DAMPRCTRL*256)
		cmp	byte ptr es:[bx+si +1].cDamprPbend,080h
		jb		local1Upd1
		mov	cl,07fh
local1Upd1:
		CallDriver DController
		mov	cx,es:[bx+si].cDamprPbend
		shr	si,1
		xchg	cl,ch
		shl	cl,1
		cmp	ch,080h
		jb		local2Upd1
		or		cl,1
local2Upd1:
		and	cx,7f7fh
		CallDriver DPitchBend
		mov	cl,es:[bx+si].cCurNote
		mov	ch,CURNOTE
		CallDriver DController

		pop	si
		pop	dx
		pop	ax
		ENDM

UpdateChannel2	MACRO							;Update a channel from node
		push	ax									;information
		push	dx
		push	si

		xor	ah,ah
		xchg	si,ax

		mov	cx,(ALLNOFF*256)
		CallDriver DController
		mov	cl,es:[bx+si].cPriVoice
		and	cl,0fh
		mov	ch,NUMNOTES
		CallDriver DController
		mov	cl,es:[bx+si].cProgram
		CallDriver DProgramChange
		push	si
		mov	si,ax
		mov	VolRequest[si],0ffh
		pop	si
		mov	cl,es:[bx+si].cVolume
		mov	dl,es:[bx].sVolume
		call	ScaleVolume
		mov	ch,VOLCTRL
		CallDriver DController
		mov	ch,PANCTRL
		mov	cl,es:[bx+si].cPan
		CallDriver DController
		mov	ch,MODCTRL
		mov	cl,es:[bx+si].cModulation
		CallDriver DController
		shl	si,1
		mov	cx,(DAMPRCTRL*256)
		cmp	byte ptr es:[bx+si + 1].cDamprPbend,080h
		jb		local1Upd2
		mov	cl,07fh
local1Upd2:
		CallDriver DController
		mov	cx,es:[bx+si].cDamprPbend
		shr	si,1
		xchg	cl,ch
		shl	cl,1
		cmp	ch,080h
		jb		local2Upd2
		or		cl,1
local2Upd2:
		and	cx,7f7fh
		CallDriver DPitchBend
		mov	cl,es:[bx+si].cCurNote
		mov	ch,CURNOTE
		CallDriver DController

		pop	si
		pop	dx
		pop	ax
		ENDM

;---------------------------------------------------------------------------
;	MIDI.S interface to C
;---------------------------------------------------------------------------

DoSound		PROC								;C INTERFACE
		push	bp
		mov	bp,sp
		push	es
		push	ds
		push	di
		push	si

                mov     SaveDS,DS
		les	ax,dword ptr [bp+6+Cparam] ;The first argument is
		mov	cx,word ptr [bp+10+Cparam] ;an address
		mov	Carg1,cx
		mov	cx,word ptr [bp+12+Cparam]
		mov	Carg2,cx
		mov	cx,word ptr [bp+14+Cparam]
		mov	Carg3,cx
		mov	cx,word ptr [bp+16+Cparam]
		mov	Carg4,cx
		mov	cx,word ptr [bp+18+Cparam]
		mov	Carg5,cx
		mov	cx,word ptr [bp+20+Cparam]
		mov	Carg6,cx

		mov	cx,word ptr [bp+4+Cparam]  ;If the function was
		cmp	cl,SMasterVol		   ;SMasterVol, SProcesss,
		je	fixFirstArg	   	   ;SSetReverb, or SSoundOn,
		cmp	cl,SProcess		   ;then the first argument
		je	fixFirstArg	   	   ;was an int value, not an
		cmp	cl,SSoundOn		   ;address
		je	fixFirstArg                
		cmp	cl,SSetReverb
		jne	doSoundCall
fixFirstArg:
		mov	dx,word ptr [bp+6+Cparam]  
		mov	Carg1,dx

doSoundCall:
		call	DispatchC
outTest:
		pop	si
		pop	di
		pop	ds
		pop	es
		pop	bp
		ret
DoSound		ENDP

DispatchC	PROC	NEAR						;BRANCH TO CORRECT PROCEDURE
		cmp	cl,SPatchReq
		jne	notPatchReq
		call	FAR PTR PatchReq
		xor	ah,ah
		cmp	al,0ffh
		jne	notNoPatch
		mov	ah,al
notNoPatch:
		xor	cx,cx
		mov	cl,numVoices
		les	bx,dword ptr Carg1
		mov	es:[bx],cx
		mov	cl,0
		les	bx,dword ptr Carg3
		mov	es:[bx],cx
		mov	cl,devID
		les	bx,dword ptr Carg5
		mov	es:[bx],cx
		ret

notPatchReq:
		cmp	cl,SInit
		jne	notInit
		mov	bx,Carg1
		call	FAR PTR Init
		ret

notInit:
		cmp	cl,STerminate
		jne	notTerminate
		call	FAR PTR Terminate
		ret

notTerminate:
		cmp	cl,SProcess
		jne	notProcess
		mov	cx,Carg1
		call	FAR PTR ProcessSounds
		ret

notProcess:
		cmp	cl,SSoundOn
		jne	notSoundOn
		mov	cx,Carg1
		call	FAR PTR SoundOn
		ret

notSoundOn:
		cmp	cl,SRestore
		jne	notRestore
		call	FAR PTR RestoreSound
		ret

notRestore:
		cmp	cl,SMasterVol
		jne	notMasterVol
		mov	cx,Carg1
		call	FAR PTR MasterVol
		ret

notMasterVol:
		cmp	cl,SSetReverb
		jne	notSetReverb
		mov	cx,Carg1
		call	FAR PTR SetReverb
		ret

notSetReverb:
		cmp	cl,SPlay
		jne	notPlay
		mov	cx,Carg1							;*** That funky bed sound thing
		call	FAR PTR PlaySound
		ret

notPlay:
		cmp	cl,SEnd
		jne	notEnd
		call	FAR PTR EndSound
		ret

notEnd:
		cmp	cl,SPause
		jne	notPause
		mov	cl,byte ptr Carg1
		call	FAR PTR PauseSound
		ret

notPause:
		cmp	cl,SFade
		jne	notFade
		mov	dl,byte ptr Carg1
		mov	cl,byte ptr Carg2
		mov	ch,byte ptr Carg3
		call	FAR PTR FadeSound
		ret

notFade:
		cmp	cl,SHold
		jne	notSHold
		mov	cl,byte ptr Carg1
		call	FAR PTR HoldSound
		ret

notSHold:
		cmp	cl,SMute
		jne	notSMute
		mov	cx,Carg1
		call	FAR PTR MuteSound
		ret

notSMute:
		cmp	cl,SChangeVol
		jne	notChangeVol
		mov	cx,Carg1
		call	FAR PTR ChangeVol
		ret

notChangeVol:
		cmp	cl,SChangePri
		jne	notChangePri
		mov	cx,Carg1
		call	FAR PTR ChangePri
					
notChangePri:
		cmp	cl,SGetDataInc
		jne	notGetData
		call	FAR PTR GetDataInc
		mov	ax,cx

notGetData:
		cmp	cl,SGetSignal
		jne	notGetSignal
		call	FAR PTR GetSignal
		mov	al,cl
		xor	ah,ah
		ret

notGetSignal:
		cmp	cl,SGetSignalRset
		jne	notGetSignalRset
		call	FAR PTR GetSignalRset
		mov	al,cl
		xor	ah,ah
		ret

notGetSignalRset:
		cmp	cl,SGetSYMPTE
		jne	notGetSMPTE
		call	FAR PTR GetSMPTE
		xor	ah,ah
		les	bx,dword ptr Carg1
		mov	al,dl
		mov	es:[bx],ax
		les	bx,dword ptr Carg3
		mov	al,ch
		mov	es:[bx],ax
		les	bx,dword ptr Carg5
		mov	al,cl
		mov	es:[bx],ax

notGetSMPTE:
		cmp	cl,SNoteOff
		jne	notSNoteOff
		mov	dl,byte ptr Carg1
		mov	ch,byte ptr Carg2
		mov	cl,byte ptr Carg3
		call	FAR PTR SendNoteOff
		ret

notSNoteOff:
		cmp	cl,SNoteOn
		jne	notSNoteOn
		mov	dl,byte ptr Carg1
		mov	ch,byte ptr Carg2
		mov	cl,byte ptr Carg3
		call	FAR PTR SendNoteOn
		ret

notSNoteOn:
		cmp	cl,SController
		jne	notSContrlr
		mov	dl,byte ptr Carg1
		mov	ch,byte ptr Carg2
		mov	cl,byte ptr Carg3
		call	FAR PTR SendContrlr
		ret

notSContrlr:
		cmp	cl,SPChange
		jne	notSPChange
		mov	dl,byte ptr Carg1
		mov	cl,byte ptr Carg2
		call	FAR PTR SendPChange
		ret

notSPChange:
		cmp	cl,SPBend
		jne	notSPbend
		mov	dl,byte ptr Carg1
		mov	cx,word ptr Carg2
		call	FAR PTR SendPBend
		ret

notSPbend:

ifdef RECMID
		cmp	cl,SAskDriver
		jne	notSAskDriver
		mov	ah,byte ptr Carg1
		mov	al,byte ptr Carg2
		mov	ch,byte ptr Carg3
		mov	cl,byte ptr Carg4
		call	FAR PTR AskDriver
notSAskDriver:
endif

		ret
DispatchC	ENDP

;---------------------------------------------------------------------------
;
;---------------------------------------------------------------------------

PatchReq	PROC	FAR							;DETERMINE PATCH REQUIREMENTS
		mov	word ptr Driver,ax
		mov	word ptr Driver+2,es

; @INT - Set address of StashMIDI routine for driver to call
ifdef RECMID
                mov     ax,cs
                mov     es,ax
                mov     ax,offset StashMIDI
endif
                
		CallDriver DPatchReq
		mov	numVoices,cl
		mov	devID,ch
		
                ;Removed DAC code @DAC

		ret
PatchReq	ENDP



Init		PROC	FAR							;INITIALIZE SOUND DRIVER
		push	cx

		CallDriver DInit						;Call Init function of
		mov	loChnl,cl						;Driver and set loChnl &
		mov	hiChnl,ch						;hiChnl with return value

		push	ax
		xor	cl,cl								;Reset reverb mode
		CallDriver DSetReverb
		pop	ax

		pop	cx
		ret
Init		ENDP


Terminate	PROC	FAR						;TERMINATE DRIVER
		push	bx
		push	cx
		push	es
		push	si

;**		xor	si,si							;If any sounds are
;**loopTrmNodes:
;**		cmp	dword ptr PlayList[si],0
;**		je		noTermNode
;**		les	ax,PlayList[si]			;still playing, kill them
;**		call	EndSound
;**noTermNode:
;**		add	si,4
;**		cmp	si,64
;**		jne	loopTrmNodes

		mov	cl,15								;Leave the devices volume
		CallDriver DMasterVol				;at maximum

		CallDriver DTerminate				;Terminate the driver
					
ifdef RECMID									;*** If UART was turned on,
		call	ResetMPU	   ;Reset the MPU
                call    Drop_M_Interrupt   ;Remove MIDI interrupt - @INT 
endif

outTerminate:
		pop	si
		pop	es
		pop	cx
		pop	bx
		ret
Terminate	ENDP



ProcessSounds	PROC	FAR	    	;INC OR DEC PROCESS COUNTER
		cmp	cx,0
		jne	processTrue
		inc	processSnds
		ret

processTrue:
		cmp	processSnds,0
		je	processExit
		dec	processSnds
processExit:
		ret
ProcessSounds	ENDP


SoundOn		PROC	FAR						;TURN SOUND ON OR OFF
		CallDriver DSoundOn
		ret
SoundOn		ENDP


RestoreSound	PROC	FAR					;PUT SOUND BACK ON PLAYLIST
		pushf
		cli
		push	si
		push	ax
		push	bx
		push	cx
		push	dx

		mov	bx,ax
		assume	bx:ptr Sound

		xor	ch,ch								;** That funky bed sound thing
		mov	cl,es:[bx].sState
		dec	cl

		mov	si,14								;Save the values of the node
loopSaveNode:
		mov	dl,es:[bx+si].cMute			;which will be overwritten
		and	dl,0f0h							;by the restore, and that
		mov	savMuteFlag[si],dl			;must remain intact
		dec	si
		jns	loopSaveNode
		mov	dl,es:[bx].sHold			;which will be overwritten
                mov     Holding,dl

		mov	restoring,1						;Put it on the PlayList
		call	FAR PTR PlaySound

		mov	cx,  es:[bx].sTimer	  			;Reset timer and set Loop
		mov	word ptr es:[bx].sTimer,0	;value
		mov	al,es:[bx].sLoop
		mov	byte ptr es:[bx].sLoop,1

		cmp	cx,0
		je		noRestore
loopRest:
		mov	dx,es:[bx].sTimer				;Fast forward through sound
		call	ParseNode						;to the point where it was
		cmp	dx,es:[bx].sTimer				;saved
		jb		noWrap
		je		noRestore
		sub	dx,es:[bx].sTimer
		sub	cx,dx
noWrap:
		cmp	cx, es:[bx].sTimer
		jne	loopRest

noRestore:
		mov	es:[bx].sLoop,al				;Put old loop value back, and
		mov	restoring,0						;clear restore flag

		mov	si,14								;Restore the values of the
loopRestNode:
		mov	dl,savMuteFlag[si]			;node that were previously
		or		es:[bx+si].cMute,dl			;saved
		dec	si
		jns	loopRestNode
                mov     dl,Holding
                mov     es:[bx].sHold,dl

		call	DoChannelList

		pop	dx
		pop	cx
		pop	bx
		pop	ax
		pop	si
		popf
		ret
assume bx:nothing
RestoreSound	ENDP


MasterVol	PROC	FAR			        ;CHANGE MASTER VOLUME
		cmp	cl,255				;If the volume is 255, then
		je		volOK			;we just want a return value
                                                        
		cmp	cl,15				;Insure that the volume is
		jbe	volOK				;15 or less
		mov	cl,15

volOK:
		CallDriver DMasterVol

		ret
MasterVol	ENDP


SetReverb	PROC	FAR				;CHANGE REVERB MODE
		cmp	cl,255				;If the mode is 255, then
		jne	notAskRevMode			;we just want a return value
		CallDriver DSetReverb
		ret

notAskRevMode:
		cmp	cl,10				;Insure that reverb mode is
		jbe	revOK				;10 or less.  If it's not,
		mov	al,revDefault			;then return the current
		xor	ah,ah				;default
		ret

revOK:
		mov	al,revDefault			;See if the first node on the
		push	es				;PlayList is set to default
		push	ax				;reverb.  If so, tell the
		push	bx				;driver to change the reverb
		push	dx				;mode
		mov	revDefault,cl		
		les	bx,PlayList
		mov	dx,es
		or		dx,bx
		jz		outSetReverb
		cmp  (Sound ptr	es:[bx]).sReverbMode,07fh
		jne	outSetReverb
		CallDriver DSetReverb

outSetReverb:
		pop	dx
		pop	bx
		pop	ax
		pop	es
		ret
SetReverb	ENDP


PlaySound	PROC	FAR				;ADD SOUND TO PLAYLIST
assume	bx:ptr Sound
		pushf
		cli
		push	cx

		push	cx
		mov	cx,es			 	;If node es:ax is already
		xor	di,di			 	;on the PlayList, then
PfindNode:
		cmp	word ptr PlayList[di],ax	;do an End before adding
		jne	notThisNode		 	;it again
		cmp	word ptr PlayList[di+2],cx
		jne	notThisNode
		call	DoEnd
		call	DoChannelList
		jmp	SHORT initNode
notThisNode:
		add	di,4
		cmp	di,(LISTSIZE * 4)
		jne	PfindNode

initNode:
		pop	cx

		mov	bx,ax			;Set the state property of
		mov	es:[bx].sState,1     	;the node to TRUE (which
						;means the sound is playing)


		cmp	cx,0			;** That funky bed sound thing
		je	itsASFX
		inc	es:[bx].sState
itsASFX:
		call	FixupHeader		;Set up the resource header

		mov	bx,ax			;Initialize all index, rest,
		xor	si,si			;PriVoice, and channel
		xor	cx,cx			;parameters
		dec	ch
notTr16:
		shl	si,1			
		mov	word ptr es:[bx+si].tIndex,13
		mov	word ptr es:[bx+si].tLoopPoint,3
		mov	word ptr es:[bx+si].tRest,0
		mov	word ptr es:[bx+si].tLoopRest,0
		mov	word ptr es:[bx+si].cDamprPbend,2000h
		shr	si,1
		mov	es:[bx+si].tChannel,ch
		mov	es:[bx+si].tCommand,cl
		mov	es:[bx+si].tLoopCommand,cl
		mov	es:[bx+si].cPriVoice,ch
		mov	es:[bx+si].cModulation,cl
		mov	es:[bx+si].cProgram,ch
		mov	es:[bx+si].cVolume,ch
		mov	es:[bx+si].cPan,ch
		mov	es:[bx+si].cCurNote,ch
		mov	es:[bx+si].cFlags,cl
		mov	es:[bx+si].cMute,cl
		inc	si
		cmp	si,15
		jne	notTr16
		mov	es:[bx+si].tChannel,ch
		mov	es:[bx+si].tCommand,cl
		mov	es:[bx+si].tLoopCommand,cl
		mov	es:[bx].sSample,cl
		mov	es:[bx].sHold,cl
		mov	byte ptr es:[bx].sReverbMode,07fh
		shl	si,1
		mov	word ptr es:[bx+si].tIndex,13
		mov	word ptr es:[bx+si].tLoopPoint,3
		mov	word ptr es:[bx+si].tRest,0

		mov	word ptr es:[bx].sLoopTime,0

		push	ax									;Get resource
		push	bp									;pointer from sound
		push	ds									;node
		xor	si,si

ifdef RECMID			      	       	;If the resource
		lds	bp,dword ptr es:[bx].sPointer ;pointer is ffff:ffff,
		mov	dx,ds		        ;then the sound data
		cmp	dx,0ffffh		;should be taken from
		jne	playDeRef		;the MIDI port, rather
		cmp	bp,0ffffh		;than from memory
		jne	playDeRef		      
		mov	oldRecCom,0
		cmp	UARTmode,0
		jne	UARTon
		mov	UARTmode,1
UARTon:
		push	bx
		mov	bx,0ffh        
		call	MPUcomm
		mov	bx,03fh
		call	MPUcomm
                call    Set_M_Interrupt     ;Install MIDI interrupt - @INT
		pop	bx
		pop	ds
		pop	bp
		pop	ax
		jmp	resetCritical
endif

playDeRef:
		Hnd2Ptr es:[bx].sPointer	;De-reference resource ptr

		cmp	byte ptr ds:[bp+32],-1	;If there is priority
		je	initTheTracks	        ;information in the resource,
		cmp	es:[bx].sFixedPri,0	;and the sFixedPri flag is
		jne	initTheTracks		;not set, then set the
		mov	cl,ds:[bp+32]		;sPriority property to the
		mov	es:[bx].sPriority,cl	;priority in the resource

initTheTracks:
		mov	cx,bp			;Set channel values and rest
notTch16:
		mov	dx,ds:[bp]		;for tracks. Set PriVoice,
		cmp	dx,0			;Volume and Pan for channels.
		je	endChnl 	
		mov	bp,cx			;node to the sample track + 1
		add	bp,dx
		mov	dl,byte ptr ds:[bp]

                .if dl == 0feh
ifndef RECMID			      	       	;If the resource
                        mov     ds,SaveDS
		        invoke Panic, E_SAMPLE_IN_SND
endif
                        shl     si,1
                        mov     word ptr es:[bx+si].tIndex,0
                        mov     word ptr es:[bx+si].tLoopPoint,0
                        shl     si,1
                        mov     byte ptr es:[bx+si].tChannel,0feh
                        jmp     nextInitChnl2
                .endif

                ;Removed DAC code @DAC

		mov	es:[bx+si].tChannel,dl
		mov	dh,dl
		or	dh,0b0h
		mov	es:[bx+si].tCommand,dh
		mov	dl,byte ptr ds:[bp+12]
		xor	dh,dh
		cmp	dl,TIMINGOVER
		jne	notTOver
		mov	dl,0f0h
		mov	dh,080h
notTOver:
		shl	si,1
		mov	es:[bx+si].tRest,dx
		shr	si,1
		push	si
		mov	dl,es:[bx+si].tChannel
		and	byte ptr es:[bx+si].tChannel,0fh
		test	dl,16
		jz	notGhostTrack
		shl	si,1
		mov	word ptr es:[bx+si].tIndex,3
		mov	word ptr es:[bx+si].tRest,0
		xor	dh,dh
		mov	si,dx
		and	si,0fh
		or		byte ptr es:[bx+si].cFlags,2
		jmp	SHORT nextInitChnl
notGhostTrack:
		xor	dh,dh
		mov	si,dx
		and	si,0fh
		test	dl,32
		jz	notLockedCh
		or	es:[bx+si].cFlags,1
notLockedCh:
		test	dl,64
		jz	notMutedCh
		mov	es:[bx+si].cMute,1
notMutedCh:
		cmp	si,15
		jne	notInitCh16
		cmp	es:[bx].sReverbMode,07fh
		jne	noPVol
		mov	al,ds:[bp+8]
		mov	es:[bx].sReverbMode,al
		jmp	SHORT nextInitChnl
notInitCh16:
		cmp	byte ptr es:[bx+si].cPriVoice,0ffh
		jne	noPPri
		mov	al,byte ptr ds:[bp+1]
		mov	es:[bx+si].cPriVoice,al
noPPri:
		cmp	byte ptr es:[bx+si].cProgram,0ffh
		jne	noPPrg
		mov	al,ds:[bp+4]
		mov	es:[bx+si].cProgram,al
noPPrg:
		cmp	byte ptr es:[bx+si].cVolume,0ffh
		jne	noPVol
		mov	al,ds:[bp+8]
		mov	es:[bx+si].cVolume,al
noPVol:
		cmp	byte ptr es:[bx+si].cPan,0ffh
		jne	nextInitChnl
		mov	al,ds:[bp+11]
		mov	es:[bx+si].cPan,al
nextInitChnl:
		pop	si
nextInitChnl2:
		inc	si
		shl	si,1
		mov	bp,cx
		add	bp,si
		shr	si,1
		cmp	si,16
endChnlje:
		je	endChnl
		jmp	notTch16
endChnl:
		pop	ds
		pop	bp
		pop	ax

resetCritical:
		cmp	es:[bx].sState,2     ;** That funky bed sound thing
		jne	notFunkyBed
		mov	di,14
loopFunkyBed:
		or	es:[bx+di].cFlags,1
		dec	di
		jns	loopFunkyBed
notFunkyBed:
		mov	ax,bx			   				;Find a spot on the
		mov	dl,es:[bx].sPriority	   	;PlayList to append or
		push	es									;insert the new sound node.
		xor	di,di			   				;If there is no available
PfindPri:										;spot, and all sounds on
		cmp	word ptr PlayList[di+2],0	;the PlayList are of a
		je		placeHere						;higher priority, then
		les	bx,PlayList[di]				;exit the procedure
		cmp	es:[bx].sPriority,dl
		jbe	insertHere
		add	di,4
		cmp	di,(LISTSIZE * 4)
		jne	PfindPri
		pop	es
		jmp	SHORT outPlay

insertHere:
		mov	si,((LISTSIZE - 2) * 4)	;If the spot was in the
keepInsrting:
		mov	bx,si								;middle of the play list,
		add	bx,4								;then move everyone down one
		cmp	bx,di
		je		placeHere
		mov	bx,word ptr PlayList[si]				
		mov	word ptr PlayList[si+4],bx				
		mov	bx,word ptr PlayList[si+2]
		mov	word ptr PlayList[si+6],bx
		sub	si,4
		jmp	SHORT keepInsrting

placeHere:
		pop	es
		mov	bx,ax
		mov	ax,es
		mov	word ptr PlayList[di],bx	;Place node on play list
		mov	word ptr PlayList[di+2],ax	;where di is pointing to

		cmp	restoring,0						;If this sound is restoring,
		jnz	outPlay							;don't reset critical values
		xor	cx,cx
		mov	es:[bx].sDataInc,cx
		mov	es:[bx].sTimer,cx
		mov	es:[bx].sSignal,cl
		mov	es:[bx].sFadeDest,cl
		mov	es:[bx].sFadeTicks,cl
		mov	es:[bx].sFadeCount,cl
		mov	es:[bx].sFadeSteps,cl
		mov	es:[bx].sPause,cl

		call	DoChannelList					;Update ChList

outPlay:
		pop	cx
		popf
		ret
assume bx:nothing
PlaySound	ENDP


EndSound	PROC	FAR							;END A SOUND
		pushf
		cli

		call	DoEnd								;Take node off PlayList, and
		call	DoChannelList					;update ChList

		popf
		ret
EndSound	ENDP


PauseSound	PROC	FAR						;PAUSE/UNPAUSE A SOUND
assume	bx:ptr Sound
		pushf
		cli
		push	ax
		push	bx
		push	si

		mov	bx,es								;If the node pointer is set
		or		bx,ax								;to 0000:0000, then pause all
		jne	pauseOne							;sounds on the PlayList

		xor	si,si								;For every sound node on
lookPause:
		les	bx,PlayList[si]				;the PlayList, increment or
		mov	ax,es								;decrement the sPause
		or		ax,bx								;property
		jne	goodPauseNode
		cmp	si,0
		jne	outPause
		mov	si,4
		jmp	SHORT lookPause
goodPauseNode:
		mov	al,es:[bx].sPause
		cmp	cl,0
		je		unPause
		inc	al
		jmp	SHORT storePause
unPause:
		cmp	al,0
		je		storePause
		dec	al
storePause:
		mov	es:[bx].sPause,al
		add	si,4
		cmp	si,(LISTSIZE * 4)
		jne	lookPause
		jmp	SHORT outPause
					
pauseOne:
		mov	bx,ax								;Increment or decrement the
		mov	al,es:[bx].sPause				;sPause property of the
		cmp	cl,0								;sound node specified
		je		unPauseOne
		inc	al
		jmp	SHORT storeOnePause
unPauseOne:
		cmp	al,0
		je		storeOnePause
		dec	al
storeOnePause:
		mov	es:[bx].sPause,al

outPause:
		call	DoChannelList
		pop	si
		pop	bx
		pop	ax
		popf
		ret
assume bx:nothing
PauseSound	ENDP


FadeSound	PROC	FAR						;FADE A SOUND TO GIVEN LEVEL
assume	bx:ptr Sound
		pushf
		cli
		push	bx

		mov	bx,ax								;If the volume of the node
		cmp	es:[bx].sVolume,dl			;is already there, then
		je		outFade							;don't bother

		mov	es:[bx].sFadeDest,dl			;Set fade properties in the
		mov	es:[bx].sFadeTicks,cl		;sound node
		mov	es:[bx].sFadeSteps,ch			
		mov	byte ptr es:[bx].sFadeCount,0

outFade:
		pop	bx
		popf
		ret
assume bx:nothing
FadeSound	ENDP


HoldSound	PROC	FAR						;HOLD/RELEASE A SOUND
		push	bx

		mov	bx,ax								;Set sHold property of node
		mov	(Sound ptr es:[bx]).sHold,cl

		pop	bx
		ret
HoldSound	ENDP


MuteSound	PROC	FAR						;MUTE/UNMUTE AN ENTIRE NODE
assume	bx:ptr Sound
		push	bx
		push	dx
		push	si
		
		inc	processSnds

		mov	bx,ax
		mov	si,14
loopMute:										;Change the mute flag of
		mov	dl,es:[bx+si].cMute			;every channel in the node
		cmp	cx,0								;Decide whether to mute or
		jne	muteInc							;unmute the node
		cmp	dl,00fh
		jbe	storeMute
		sub	dl,10h
		jmp	SHORT storeMute
muteInc:
		cmp	dl,0f0h
		jae	storeMute
		add	dl,10h
storeMute:
		mov	es:[bx+si].cMute,dl
		dec	si
		jns	loopMute

		call	DoChannelList

		dec	processSnds

		pop	si
		pop	dx
		pop	bx
		ret
assume bx:nothing
MuteSound	ENDP


ChangeVol	PROC	FAR						;CHANGE SOUND NODE VOLUME
		pushf
		cli
		push	bx

		mov	bx,ax
		call	FindNode
		xor	ch,ch
		call	DoChangeVol

		pop	bx
		popf
		ret
ChangeVol	ENDP


ChangePri	PROC	FAR						;CHANGE SOUND NODE PRIORITY
assume	bx:ptr Sound
		pushf
		cli
		push	bx
		push	cx
		push	dx
		push	si
		push	di

		mov	bx,ax								;If it's the same as before,
		cmp	es:[bx].sPriority,cl			;then don't bother
		jne	doPri
		jmp	outChPri

doPri:
		mov	es:[bx].sPriority,cl			;Store new priority

		call	FindNode							;If it's not on the PlayList
		cmp	si,255							;then exit the procedure
		jne	movePriNodes
		jmp	outChPri

movePriNodes:
		mov	word ptr PlayList[si],0		;Remove the node from the
		mov	word ptr PlayList[si+2],0	;PlayList
		cmp	si,((LISTSIZE - 1) * 4)
		je		CPaddNode
CPmoveNodes:
		mov	cx,word ptr PlayList[si+4]
		mov	word ptr PlayList[si],cx
		mov	cx,word ptr PlayList[si+6]
		mov	word ptr PlayList[si+2],cx
		add	si,4
		cmp	si,((LISTSIZE - 1) * 4)
		jne	CPmoveNodes
		mov	word ptr PlayList[si],0
		mov	word ptr PlayList[si+2],0

CPaddNode:
		mov	dl,es:[bx].sPriority			;Put node back on in order of priority
		push	es
		xor	di,di								
CPfindPri:
		cmp	word ptr PlayList[di+2],0
		je		CPplaceHere
		les	bx,PlayList[di]
		mov	es:[bx].sPriority,dl
		jbe	CPinsertHere
		add	di,4
		jmp	SHORT CPfindPri
CPinsertHere:
		mov	si,((LISTSIZE - 2) * 4)
insertMore:
		mov	bx,si
		add	bx,4
		cmp	bx,di
		je		CPplaceHere
		mov	bx,word ptr PlayList[si]				
		mov	word ptr PlayList[si+4],bx				
		mov	bx,word ptr PlayList[si+2]
		mov	word ptr PlayList[si+6],bx
		sub	si,4
		jmp	SHORT insertMore
CPplaceHere:
		pop	es
		mov	word ptr PlayList[di],ax
		mov	cx,es
		mov	word ptr PlayList[di+2],cx

		call	DoChannelList					;Update ChList

outChPri:
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		popf
		ret
assume 	bx:nothing
ChangePri	ENDP


GetSignal	PROC	FAR						;GET SIGNAL VALUE OF A NODE
		push	bx

		mov	bx,ax								;Get signal and put it in cl
		mov	cl,(Sound ptr es:[bx]).sSignal

		pop	bx
		ret
GetSignal	ENDP


GetSignalRset	PROC	FAR					;GET SIGNAL VALUE OF A NODE
		push	bx

		mov	bx,ax								;Get signal and put it in cl
		xor	cl,cl								;and reset signal
		xchg	cl,(Sound ptr es:[bx]).sSignal

		pop	bx
		ret
GetSignalRset	ENDP


GetDataInc	PROC	FAR						;GET DATAINC VALUE OF A NODE
		push	bx

		mov	bx,ax								;Get DataInc value and put it
		mov	cx,(Sound ptr es:[bx]).sDataInc			;in cx

		pop	bx
		ret
GetDataInc	ENDP


GetSMPTE	PROC	FAR							;GET SMPTE VALUE OF A NODE
		push	ax
		push	bx

		mov	bx,ax								;Put the sTimer value of the
		mov	ax,(Sound ptr es:[bx]).sTimer				;node into ax

		xor	dx,dx								;Convert to minutes
		mov	cx,3600
		div	cx
		push	ax

		mov	ax,dx								;Convert to seconds
		mov	cl,60								;and frames
		div	cl

		mov	cl,ah
		shr	cl,1
		mov	ch,al
		pop	dx

		pop	bx
		pop	ax
		ret
GetSMPTE	ENDP


;---------------------------------------------------------
;SendNoteOff - Send a NoteOff to a given channel in
;              a given node
;
; in  - es:ax = far node ptr
;       ch    = note #
;       cl    = velocity
;       dl    = channel
;
; out - nothing
;---------------------------------------------------------
SendNoteOff	PROC	FAR
assume	bx:ptr Sound
		push	ax
		push	bx
		push	dx
		push	si


		;Disable the SoundServer

		inc	processSnds


		;Put the position of the node on the PlayList
		;into si.  If 255 is returned, then the node
		;is not on the PlayList at all

		mov	bx,ax
		call	FindNode
		.if	si != 255

			push	si


			;Reset the CurNote

			xor	dh,dh
			mov	si,dx
			mov	es:[bx+si].cCurNote,-1

			
ifdef RECMID
			push	es
			push	ax
			push	cx
			
			les	ax,es:[bx].sPointer
			mov	cx,es
			or		ax,cx
			.if ax == -1
				mov	si,dx
				pop	cx
				pop	ax
				pop	es
				CallDriver DNoteOff
			 	jmp	SHORT outSendNoff
			.endif

			pop	cx
			pop	ax
			pop	es
endif


			;Combine the PlayList position and the channel
			;number in dl (this byte will represent the
			;node/channel we are sending to)

			mov	ax,si
			pop	si
			mov	dx,si
			shl	dl,1
			shl	dl,1
			or		dl,al


			;Search for an entry for this node/channel on
			;the ChList.  The position of this entry on
			;the list will tell us the physical channel that
			;this channel was mapped to.  If no such entry
			;exists, then this channel is not currently
			;being played (possibly preempted)

			xor	si,si
			.while  ChList[si] != dl

				inc	si
				cmp	si,16
				je		outSendNoff
			.endw
			

			;Set up the input registers and call the Sound
			;Driver's NoteOff function

			mov	ax,si
			CallDriver DNoteOff
		.endif


outSendNoff:

		;Enable the SoundServer

		dec	processSnds


		pop	dx
		pop	bx
		pop	ax
		pop	si
		ret
assume bx:nothing
SendNoteOff	ENDP



;---------------------------------------------------------
;SendNoteOn - Send a NoteOn to a given channel in
;             a given node
;
; in  - es:ax = far node ptr
;       ch    = note #
;       cl    = velocity
;       dl    = channel
;
; out - nothing
;---------------------------------------------------------
SendNoteOn	PROC	FAR
assume	bx:ptr Sound
		push	ax
		push	bx
		push	dx
		push	si


		;Disable the SoundServer

		inc	processSnds


		;Put the position of the node on the PlayList
		;into si.  If 255 is returned, then the node
		;is not on the PlayList at all

		mov	bx,ax
		call	FindNode
		.if si != 255

			push	si


			;Fill the node's CurNote field with
			;the note number specified

			xor	dh,dh
			mov	si,dx
			mov	es:[bx+si].cCurNote,ch

			
ifdef RECMID
			push	es
			push	ax
			push	cx
			 
			les	ax,es:[bx].sPointer
			mov	cx,es
			or		ax,cx
			.if ax == -1
			 	mov	si,dx
				pop	cx
				pop	ax
				pop	es
				CallDriver DNoteOn
			 	jmp	SHORT outSendNon
			.endif

			pop	cx
			pop	ax
			pop	es
endif


			;Combine the PlayList position and the channel
			;number in dl (this byte will represent the
			;node/channel we are sending to)

			mov	ax,si
			pop	si
			mov	dx,si
			shl	dl,1
			shl	dl,1
			or		dl,al


			;Search for an entry for this node/channel on
			;the ChList.  The position of this entry on
			;the list will tell us the physical channel that
			;this channel was mapped to.  If no such entry
			;exists, then this channel is not currently
			;being played (possibly preempted)

			xor	si,si
			.while ChList[si] != dl

				inc	si
				cmp	si,16
				je		outSendNon
			.endw
			

			;Set up the input registers and call the Sound
			;Driver's NoteOff function

			mov	ax,si
			CallDriver DNoteOn
		.endif


outSendNon:

		;Enable the SoundServer

		dec	processSnds


		pop	dx
		pop	bx
		pop	ax
		pop	si
		ret
assume bx:nothing
SendNoteOn	ENDP


SendContrlr	PROC	FAR						;SEND CONTROLLER TO A NODE
assume	bx:ptr Sound
		pushf
		cli
		push	ax
		push	bx
		push	cx
		push	dx
		push	si

		mov	bx,ax								;If the node isn't on the
		call	FindNode							;PlayList, exit the procedure
		cmp	si,255
		jne	gotContrlrNode
		jmp	outSCtrlr

gotContrlrNode:
		push	si
		xor	dh,dh
		mov	si,dx
					
		cmp	ch,VOLCTRL						;If it's one of the four
		jne	notSCvol							;controllers we save, then
		mov	es:[bx+si].cVolume,cl		;save it before sending it
		mov	dl,es:[bx].sVolume
		call	ScaleVolume
		jmp	SHORT sendContrlr
notSCvol:
		cmp	ch,PANCTRL
		jne	notSCPan
		mov	es:[bx+si].cPan,cl
		jmp	SHORT sendContrlr
notSCPan:
		cmp	ch,MODCTRL
		jne	notSCMod
		mov	es:[bx+si].cModulation,cl
		jmp	SHORT sendContrlr
notSCMod:
		cmp	ch,DAMPRCTRL
		jne	notSDamp
		shl	si,1
		mov	ax,es:[bx+si].cDamprPbend
		and	ah,7fh
		cmp	cl,0
		je		SCdampUp
		or		ah,80h
SCdampUp:
		mov	es:[bx+si].cDamprPbend,ax
		shr	si,1
		jmp	SHORT sendContrlr
notSDamp:
		cmp	ch,MUTECTRL
		jne	notSCMute
		push	dx
		mov	dl,es:[bx+si].cMute
		cmp	cl,0
		jne	incMuteFlag
		cmp	dl,00fh
		jbe	outMuteFlag
		sub	dl,010h
		jmp	SHORT storeMuteFlag
incMuteFlag:
		cmp	dl,0f0h
		jae	outMuteFlag
		add	dl,010h
storeMuteFlag:
		mov	es:[bx+si].cMute,dl
		call	DoChannelList
outMuteFlag:
		pop	dx
		pop	si
		jmp	SHORT outSCtrlr
notSCMute:
		cmp	ch,127
		jne	sendContrlr
		mov	es:[bx+si].cProgram,cl


sendContrlr:	ifdef	RECMID
		 push	es
		 push	ax
		 push	cx
		 les	ax,es:[bx].sPointer
		 mov	cx,es
		 cmp	cx,0ffffh
		 jne	notRecContrlr
		 cmp	ax,0ffffh
		 jne	notRecContrlr
		 pop	cx
		 pop	ax
		 pop	es
		 pop	si
		 mov	si,dx
		 and	si,0ffh
		 jmp	SHORT SConList
notRecContrlr:	 pop	cx
		 pop	ax
		 pop	es
		endif


		mov	ax,si
		pop	si
		mov	dx,si
		shl	dl,1
		shl	dl,1
		or		dl,al

		xor	si,si								;Look for the node/channel
SClookList:
		cmp	ChList[si],dl					;on the ChList.  If it's not
		je		SConList							;there, then exit
		inc	si
		cmp	si,16
		jne	SClookList
		jmp	SHORT outSCtrlr

SConList:
		mov	ax,si
		cmp	ch,127
		jne	itsAContrlr
		CallDriver DProgramChange
		jmp	SHORT outSCtrlr
itsAContrlr:
		CallDriver DController

outSCtrlr:
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		popf
		ret
assume bx:nothing
SendContrlr	ENDP


;---------------------------------------------------------
;SendPChange - Send a ProgramChange to a given channel in
;              a given node
;
; in  - es:ax = far node ptr
;       cl    = program #
;       dl    = channel
;
; out - nothing
;---------------------------------------------------------
SendPChange	PROC	FAR
assume bx:ptr Sound
		push	ax
		push	bx
		push	dx
		push	si


		;Disable the SoundServer

		inc	processSnds


		;Put the position of the node on the PlayList
		;into si.  If 255 is returned, then the node
		;is not on the PlayList at all

		mov	bx,ax
		call	FindNode
		.if si != 255

			push	si


			;Store the new program in the node

			xor	dh,dh
			mov	si,dx
			mov	es:[bx+si].cProgram,cl

			
ifdef RECMID
			push	es
			push	ax
			push	cx
			
			les	ax,es:[bx].sPointer
			mov	cx,es
			or		ax,cx
			.if ax == -1
			 	mov	si,dx
				pop	cx
				pop	ax
				pop	es
				CallDriver DProgramChange
			 	jmp	SHORT outSendPch
			.endif

			pop	cx
			pop	ax
			pop	es
endif


			;Combine the PlayList position and the channel
			;number in dl (this byte will represent the
			;node/channel we are sending to)

			mov	ax,si
			pop	si
			mov	dx,si
			shl	dl,1
			shl	dl,1
			or		dl,al


			;Search for an entry for this node/channel on
			;the ChList.  The position of this entry on
			;the list will tell us the physical channel that
			;this channel was mapped to.  If no such entry
			;exists, then this channel is not currently
			;being played (possibly preempted)

			xor	si,si
			.while ChList[si] != dl

				inc	si
				cmp	si,16
				je		outSendPch
			.endw			

			;Set up the input registers and call the Sound
			;Driver's ProgramChange function

			mov	ax,si
			CallDriver DProgramChange
		.endif


outSendPch:
		;Enable the SoundServer

		dec	processSnds


		pop	dx
		pop	bx
		pop	ax
		pop	si
		ret
assume bx:nothing
SendPChange	ENDP


SendPBend	PROC	FAR						;SEND PITCH BEND TO A NODE
assume bx:ptr Sound
		push	ax
		push	bx
		push	cx
		push	dx
		push	si

		mov	bx,ax								;If the node isn't on the
		call	FindNode							;PlayList, then exit
		cmp	si,255
		je		outSPB

		push	si									;Save the new PitchBend value
		xor	dh,dh								;in the sound node
		mov	si,dx
		shl	si,1
		mov	ax,cx
		cmp	byte ptr es:[bx+si + 1].cDamprPbend,128
		jb		noSPdampr
		or		ah,80h
noSPdampr:
		mov	es:[bx+si].cDamprPbend,ax
		shr	si,1

		mov	dx,si
		pop	si
		shl	si,1
		shl	si,1
		or		dx,si

		xor	si,si								;If the node/channel isn't on
SPlookList:
		cmp	ChList[si],dl					;the ChList, then exit
		je		SPonList
		inc	si
		cmp	si,16
		jne	SPlookList
		jmp	SHORT outSPB

SPonList:
		shl	ch,1
		cmp	cl,128
		jb		noSPhiBit
		or		ch,1
noSPhiBit:
		and	cl,7fh
		xchg	ch,cl
		mov	ax,si
		CallDriver DPitchBend

outSPB:
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret
		assume bx:nothing
SendPBend	ENDP


;---------------------------------------------------------------------------
;			   Internal Functions

DoEnd		PROC	NEAR							;END A SOUND
assume bx:ptr Sound
		push	si
		push	es
		push	ax
		push	bx
		push	ds
		push	bp

ifdef RECMID									;If it was a test node,
		push	es									;make sure any notes still
		push	ax									;playing are turned off
		mov	bx,ax
		les	ax,es:[bx].sPointer
		mov	cx,es
		cmp	cx,0ffffh
		jne	notEndRecv
		cmp	ax,0ffffh
		jne	notEndRecv
		xor	si,si
loopEndRecv:
		mov	ax,si
		mov	ch,ALLNOFF
		mov	cl,127
		CallDriver DController
		inc	si
		cmp	si,16
		jne	loopEndRecv
notEndRecv:
		pop	ax
		pop	es
endif

		xor	si,si			 					;Look for node es:ax on
		mov	cx,es			 					;the PlayList.  If it is not
findESound:
		cmp	ax,word ptr PlayList[si]	;found, then exit procedure
		jne	notEThisNode
		cmp	cx,word ptr PlayList[si+2]
		je		clearNode
notEThisNode:
		add	si,4
		cmp	si,(LISTSIZE * 4)
		jne	findESound
		jmp	outDoEnd

clearNode:
		mov	word ptr PlayList[si],0
		mov	word ptr PlayList[si+2],0	;If the node was in the
		cmp	si,((LISTSIZE -1) * 4)		;last position, then there
		je		putSignal	  					;is no need to move other nodes

moveNodes:
		mov	cx,word ptr PlayList[si+4] ;Move everybody up one
		mov	word ptr PlayList[si],cx
		mov	cx,word ptr PlayList[si+6]
		mov	word ptr PlayList[si+2],cx
		add	si,4
		cmp	si,((LISTSIZE -1) * 4)
		jne	moveNodes
		mov	word ptr PlayList[si],0
		mov	word ptr PlayList[si+2],0

putSignal:
		mov	bx,ax
		mov	byte ptr es:[bx].sSignal,255
		mov	byte ptr es:[bx].sState,0

                ;Removed DAC code 

outDoEnd:

		pop	bp
		pop	ds
		pop	bx
		pop	ax
		pop	es
		pop	si
		ret
		assume bx:nothing
DoEnd		ENDP


DoChannelList	PROC	NEAR					;UPDATE CHANNEL LIST
assume bx:ptr Sound
		push	ax									;This procedure, being the
		push	bx									;most complex and the most
		push	cx									;used, is written for speed,
		push	dx									;not for compact size.
		push	di
		push	si
		push	bp
		push	es
	
		inc	processSnds

		mov	updateChnls,0

		xor	ax,ax								;Clear channel tables
		mov	bx,-1
		mov	word ptr ChList,bx
		mov	word ptr ChList[2],bx
		mov	word ptr ChList[4],bx
		mov	word ptr ChList[6],bx
		mov	word ptr ChList[8],bx
		mov	word ptr ChList[10],bx
		mov	word ptr ChList[12],bx
		mov	word ptr ChList[14],bx
		mov	word ptr ChVoice,ax
		mov	word ptr ChVoice[2],ax
		mov	word ptr ChVoice[4],ax
		mov	word ptr ChVoice[6],ax
		mov	word ptr ChVoice[8],ax
		mov	word ptr ChVoice[10],ax
		mov	word ptr ChVoice[12],ax
		mov	word ptr ChVoice[14],ax
		mov	word ptr ChBed,ax
		mov	word ptr ChBed[2],ax
		mov	word ptr ChBed[4],ax
		mov	word ptr ChBed[6],ax
		mov	word ptr ChBed[8],ax
		mov	word ptr ChBed[10],ax
		mov	word ptr ChBed[12],ax
		mov	word ptr ChBed[14],ax
		mov	word ptr ChPri,ax
		mov	word ptr ChPri[2],ax
		mov	word ptr ChPri[4],ax
		mov	word ptr ChPri[6],ax
		mov	word ptr ChPri[8],ax
		mov	word ptr ChPri[10],ax
		mov	word ptr ChPri[12],ax
		mov	word ptr ChPri[14],ax
		mov	word ptr ChNew,bx
		mov	word ptr ChNew[2],bx
		mov	word ptr ChNew[4],bx
		mov	word ptr ChNew[6],bx
		mov	word ptr ChNew[8],bx
		mov	word ptr ChNew[10],bx
		mov	word ptr ChNew[12],bx
		mov	word ptr ChNew[14],bx

		mov	word ptr SampleList,ax		;** Sample Kludge!
		mov	word ptr SampleList[2],ax	;(until Simu-Drivers)

		les	bx,PlayList						;Set the reverb mode to the value in
		mov	dx,es								;the first node's sReverbMode value.
		or		dx,bx								;127 means default reverb mode which
		jne	notEmptyPlList					;can be set by the apps programmer.
		mov	dx,0ffffh						;empty play list
		mov	word ptr ChList,dx
		mov	word ptr ChList[2],dx		
		mov	word ptr ChList[4],dx
		mov	word ptr ChList[6],dx
		mov	word ptr ChList[8],dx
		mov	word ptr ChList[10],dx
		mov	word ptr ChList[12],dx
		mov	word ptr ChList[14],dx
		jmp	cleanUpChnls
notEmptyPlList:
		mov	cl,es:[bx].sReverbMode
		cmp	cl,127
		jne	notRevDefault
		mov	cl,revDefault
notRevDefault:
		CallDriver DSetReverb



		;*** DoChannelList PASS 1 ***


		xor	bp,bp								;Get next sound node off
		xor	si,si								;PlayList.  If its the last
		mov	al,numVoices					;one, jump out to Pass2
loopDoNodes:
		les	bx,PlayList[si]		
		mov	dx,es
		or		dx,bx
		jnz	notLastNode
		jmp	doPass2

notLastNode:
		les	bx,PlayList[si]		
		cmp	byte ptr es:[bx].sPause,0	;If the sound is paused,
		je		nodeNotPaused	  				;don't allocate any
		jmp	nextNode		  					;channels for it

nodeNotPaused:
		cmp	byte ptr es:[bx].sSample,0	;*** Sample Kludge!
		je		nodeNotSample	    			;(until Simu-Drivers
		cmp	word ptr SampleList,0		;is implemented)
		je		okSamp1
		jmp	nextNode
okSamp1:
		cmp	word ptr SampleList[2],0
		je		okSamp2
		jmp	nextNode
okSamp2:
		mov	word ptr SampleList,bx
		mov	bx,es
		mov	word ptr SampleList[2],bx
		jmp	nextNode

nodeNotSample:
		BackupList								;Backup ChannelList

		xor	di,di								;Get the channel of the next
loopDoTracks:
		mov	cl,es:[bx+di].tChannel		;track from node (Tracks
		cmp	cl,0ffh							;shoould already be
		jne	notLastTrack					;prioritized in the sound
		jmp	nextChTrack						;node

notLastTrack:
		cmp	cl,0feh							;If the channel value is
		jne	notSample						;0feh, then it's a sample.
		jmp	nextChTrack						;If it's channel 16, then
notSample:
		cmp	cl,15								;it doesn't belong on the
		jne	notCh16							;ChList.  If bit 1 of the
		jmp	nextChTrack						;cFlags array is set, then
notCh16:
		push	di									;it's a ghost channel.  If
		mov	di,cx								;bit 0 is set, then it's a
		and	di,0ffh							;locked channel.  If the
		test	es:[bx+di].cFlags,2			;cMuted property is TRUE,
		jz		notGhostChnl					;then the channel is muted
		pop	di
		jmp	nextChTrack
notGhostChnl:
		test	es:[bx+di].cMute,0ffh
		pop	di
		jz		notMutedChnl
		jmp	nextChTrack
notMutedChnl:
		mov	dx,si
		shl	dl,1
		shl	dl,1
		or		dl,cl
		push	di
		mov	di,cx
		and	di,0ffh
		mov	ah,es:[bx+di].cPriVoice
		and	ah,00fh
		mov	ch,es:[bx+di].cPriVoice
		pop	di
		shr	ch,1
		shr	ch,1
		shr	ch,1
		shr	ch,1
		jz		nonPreemptable
		push	dx
		mov	dx,16
		sub	dl,ch
		add	dx,bp
		mov	ch,dl
		pop	dx
nonPreemptable:
		push	di
		mov	di,cx
		and	di,0ffh
		test	byte ptr es:[bx+di].cFlags,1
		jz		lookOpenChnl
		cmp	ChNew[di],0ffh
		jne	lookOpenChnl
		pop	di
		mov	dh,cl
		jmp	checkVoices

lookOpenChnl:
		pop	di
		mov	dh,0ffh							;Look for an open channel.
		push	bx									;While looking, make sure
		xor	bx,bx								;that this node/channel is
loopLookChnl:
		cmp	ChNew[bx],0ffh					;not already on the list
		je		openChnl
		cmp	ChNew[bx],dl
		jne	nextLookChnl
		pop	bx
		jmp	nextChTrack
openChnl:
		cmp	bl,loChnl
		jb		nextLookChnl
		cmp	bl,hiChnl
		ja		nextLookChnl
		mov	dh,bl
nextLookChnl:
		inc	bl
		cmp	bl,16
		jne	loopLookChnl
		pop	bx
		cmp	dh,0ffh
		jne	checkVoices

		cmp	ch,0								;There were no channels
		je		gotToGetChnl					;open.  If we're a
		jmp	nextNode							;preemptable channel, then
gotToGetChnl:
		PreemptChn1								;we can blow it off and
		cmp	dh,0ffh							;proceed to the next node.
		jne	checkVoices						;If we're non-preemptable, then we
		jmp	blewIt							;have to preempt someone or back out
checkVoices:									;Got a channel
		cmp	ah,al								;Check for enough voices. If enough,
		jbe	putChOnList						;jump ahead to put channel on the list 
		cmp	ch,0								;There wern't enough voices.
		je		loopPreEmpt						;If we're preemptable, blow
		jmp	nextChTrack						;it off and go to the next track

loopPreEmpt:									;Keep preempting channels
		PreemptChn2								;until there are enough
		cmp	dh,0ffh							;voices.  If we run out of
		jne	gotSomeVoices					;preemptable channels,
		jmp	blewIt							;then we have to back out
gotSomeVoices:
		cmp	ah,al
		ja		loopPreEmpt

putChOnList:
		push	di									;Put the channel on the list
		xchg	dl,dh
		mov	di,dx
		xchg	dl,dh
		and	di,0ffh
		mov	ChNew[di],dl
		mov	ChVoice[di],ah
		sub	al,ah
		mov	ChPri[di],ch
		push	di
		mov	di,cx
		and	di,0ffh
		test	byte ptr es:[bx+di].cFlags,1
		pop	di
		jne	checkRightChnl
		mov	ChBed[di],0
		pop	di
		jmp	nextChTrack

checkRightChnl:
		mov	ChBed[di],1						;If we're a bed sound,
		cmp	dh,cl								;insure that we're on the
		jne	notRightChnl					;right channel
		pop	di
		jmp	nextChTrack

notRightChnl:
		push	si									;If the channel in our
		mov	si,cx								;desired position is not a
		and	si,0ffh							;bed channel, just swap with
		cmp	ChBed[si],0						;him.  If it is a bed...
		jne	whichBedWins
		SwapChnls
		pop	si
		pop	di
		jmp	SHORT nextChTrack

whichBedWins:
		cmp	ch,0								;If we're preemptable, we
		je		checkOtherBed					;can take ourselves off the
		mov	ChNew[di],0ffh					;list and go on to the next
		mov	ChPri[di],0						;track
		mov	ChVoice[di],0
		mov	ChBed[di],0
		add	al,ah
		pop	si
		pop	di
		jmp	SHORT nextChTrack

checkOtherBed:
		cmp	ChPri[si],0						;If the other bed channel is
		jne	preemptBed						;not preemptable, then we
		pop	si									;have to back out
		pop	di
		jmp	SHORT blewIt

preemptBed:
		add	al,ChVoice[si]					;The other bed channel was
		mov	ChNew[di],0ffh					;preemptable, so we can steal
		mov	ChVoice[di],0					;his channel
		mov	ChPri[di],0
		mov	ChBed[di],0
		mov	ChNew[si],dl
		mov	ChPri[si],ch
		mov	ChVoice[si],ah
		sub	al,ah
		pop	si
		pop	di

nextChTrack:
		inc	di									;Do the next track
		cmp	di,16
		jne	gotMoreTracks
		jmp	nextNode
gotMoreTracks:
		jmp	loopDoTracks

blewIt:
		RestoreList								;Restore the lists
nextNode:
		add	bp,16								;Increment the base priority
		add	si,4								;and do the next node
		cmp	si,(LISTSIZE*4)
		je		doPass2
		jmp	loopDoNodes



		;*** DoChannelList PASS 2 ***

doPass2:
ifdef RECMID			 						;If the RECMID option is on
		mov	ax,word ptr ChPri	 ;(for linking with the
		mov	word ptr ChPriCopy,ax	 ;tools), put the current
		mov	ax,word ptr ChPri[2]	 ;ChPri list into ChPriCopy,
		mov	word ptr ChPriCopy[2],ax ;and clear the actual list
		mov	ax,word ptr ChPri[4]	 ;so when we move channels,
		mov	word ptr ChPriCopy[4],ax ;we can move their matching
		mov	ax,word ptr ChPri[6]	 ;priorities to the same
		mov	word ptr ChPriCopy[6],ax ;position
		mov	ax,word ptr ChPri[8]
		mov	word ptr ChPriCopy[8],ax
		mov	ax,word ptr ChPri[10]
		mov	word ptr ChPriCopy[10],ax
		mov	ax,word ptr ChPri[12]
		mov	word ptr ChPriCopy[12],ax
		mov	ax,word ptr ChPri[14]
		mov	word ptr ChPriCopy[14],ax
		mov	word ptr ChPri,0
		mov	word ptr ChPri[2],0
		mov	word ptr ChPri[4],0
		mov	word ptr ChPri[6],0
		mov	word ptr ChPri[8],0
		mov	word ptr ChPri[10],0
		mov	word ptr ChPri[12],0
		mov	word ptr ChPri[14],0
endif

		xor	si,si			 ;Is the next channel a
loopPass2:
		cmp	ChNew[si],0ffh		 ;bed channel?  If so, copy
		jne	notEmptyChNew		 ;it to ChList at the same
		jmp	nextPass2		 ;position
notEmptyChNew:
		cmp	ChBed[si],0
		jne	copyBedCh
		jmp	noCopyBedCh
copyBedCh:
		xor	ax,ax			 ;It was a bed channel.  Put
		mov	al,ChNew[si]		 ;it on the list in the same
		mov	ChNew[si],0ffh		 ;position.  IF the RECMID
		mov	ChList[si],al		 ;optoin is on, move the
ifdef RECMID					 ;priority with it also
		mov	bl,ChPriCopy[si]
		mov	ChPri[si],bl
endif

		mov	di,ax			 ;See if the node/channel that
		and	di,0f0h			 ;was previously there was the
		shr	di,1			 ;same.  If not, Update the
		shr	di,1			 ;channel
		les	bx,PlayList[di]
		and	al,0fh
		cmp	ChOld[si],al
		jne	notSameBed
		mov	di,si
		shl	di,1
		shl	di,1
		cmp	word ptr ChNodes[di],bx
		jne	notSameBed
		mov	cx,es
		cmp	word ptr ChNodes[di+2],cx
		jne	notSameBed
		jmp	nextPass2
notSameBed:
		UpdateChannel1
		jmp	SHORT nextPass2

noCopyBedCh:
		mov	al,ChNew[si]            ;node/channel for this channel
		mov	bl,al
		and	ax,00fh
		and	bx,0f0h
		shr	bl,1
		shr	bl,1
		les	bx,PlayList[bx]
		mov	cl,loChnl						;Look at next node on ChNodes
		xor	ch,ch
		mov	di,cx
		shl	di,1								;list and see if it is the
		shl	di,1								;same
loopSameNode:
		cmp	word ptr ChNodes[di],bx
		jne	nextFindSame
		mov	cx,es
		cmp	word ptr ChNodes[di+2],cx
		jne	nextFindSame
		shr	di,1								;Same node, Look at the channel number
		shr	di,1								;in ChOld and see if it is the same
		cmp	ChOld[di],al
		je		sameNodeCh
		shl	di,1
		shl	di,1
nextFindSame:
		add	di,4								;If we are done looking
		mov	cx,di								;through the ChNodes list,
		shr	cl,1								;leave channel for Pass3
		shr	cl,1
		dec	cl
		cmp	hiChnl,cl
		jne	loopSameNode
		jmp	SHORT nextPass2

sameNodeCh:										;If this ChList position is going to
		cmp	ChBed[di],0						;be occupied by a bed channel,
		jne	nextPass2						;then leave this channel for Pass3

		mov	cl,ChNew[si]		;Copy this node/channel to
		mov	ChList[di],cl		;the ChList, at the position
		mov	ChNew[si],0ffh		;it used to be at.  If the
ifdef RECMID					;RECMID option is on, move
		mov	cl,ChPriCopy[si]	;the priority with it also
		mov	ChPri[di],cl
endif

nextPass2:
		inc	si									;If there are more channels
		cmp	si,16								;to copy, keep looping
		je		doPass3
		jmp	loopPass2



		;*** DoChannelList PASS 3 ***

doPass3:
		mov	al,hiChnl		;Get the next remaining
		inc	al			;channel on the ChNew list
		xor	ah,ah
		mov	di,ax
		xor	al,al
		xor	si,si
loopPass3:
		cmp	ChNew[si],0ffh
		jne	gotPass3Ch
		jmp	nextPass3
gotPass3Ch:
		mov	bx,di			;Locate the last open channel
findLastOpen:
		dec	bx			;on the ChList
		cmp	ChList[bx],0ffh
		jne	findLastOpen

		mov	di,bx			;Copy the channel to ChList,
		mov	al,ChNew[si]		;and update the channel.
		mov	ChList[di],al	        ;if the RECMID option is on,
ifdef RECMID					;move the priority with it also
		mov	bl,ChPriCopy[si]
		mov	ChPri[di],bl
endif
		mov	bl,al
		and	al,00fh
		and	bx,0f0h
		shr	bx,1
		shr	bx,1
		les	bx,PlayList[bx]
		push	si
		mov	si,di
		UpdateChannel2
		pop	si

nextPass3:
		inc	si			;Loop back to get the next
		cmp	si,16			;remaining channel
		je		cleanUpChnls
		jmp	loopPass3



		;*** CleanUp Removed Channels ***


cleanUpChnls:
		mov	si,15			;If any channels were active
loopCleanUp:
		cmp	ChOld[si],0fh		;before but are not now,
		je	nextCleanUp	        ;Turn it's notes off and
		cmp	ChList[si],0ffh		;reset it's NUMNOTES
		jne	nextCleanUp
		mov	ax,si
		mov	cx,(DAMPRCTRL*256)
		CallDriver DController
		mov	cx,(ALLNOFF*256)
		CallDriver DController
		mov	cx,(NUMNOTES*256)
		CallDriver DController
nextCleanUp:
		dec	si
		jns	loopCleanUp



		;*** Make ChOld List ***


		mov	ax,word ptr ChList	;Make the ChOld list from
		and	ax,0f0fh		;the ChList for use by the
		mov	word ptr ChOld,ax	;next call to this procudure
		mov	ax,word ptr ChList[2]
		and	ax,0f0fh
		mov	word ptr ChOld[2],ax
		mov	ax,word ptr ChList[4]
		and	ax,0f0fh
		mov	word ptr ChOld[4],ax
		mov	ax,word ptr ChList[6]
		and	ax,0f0fh
		mov	word ptr ChOld[6],ax
		mov	ax,word ptr ChList[8]
		and	ax,0f0fh
		mov	word ptr ChOld[8],ax
		mov	ax,word ptr ChList[10]
		and	ax,0f0fh
		mov	word ptr ChOld[10],ax
		mov	ax,word ptr ChList[12]
		and	ax,0f0fh
		mov	word ptr ChOld[12],ax
		mov	ax,word ptr ChList[14]
		and	ax,0f0fh
		mov	word ptr ChOld[14],ax



		;*** Make ChNodes List ***

		xor	si,si			;Construct the ChNodes list from the
		xor	di,di			;ChList and the PlayList for use in
loopChNodes:					;the next call to this procedure
		mov	bl,ChList[si]
		cmp	bl,0ffh
		jne	validChNode
		mov	word ptr ChNodes[di],0
		mov	word ptr ChNodes[di+2],0
		jmp	SHORT nextChNode
validChNode:
		and	bx,0f0h
		shr	bx,1
		shr	bx,1
		mov	ax,word ptr PlayList[bx]
		mov	word ptr ChNodes[di],ax
		mov	ax,word ptr PlayList[bx+2]
		mov	word ptr ChNodes[di+2],ax
nextChNode:
		add	di,4
		inc	si
		cmp	si,16
		jne	loopChNodes

ifdef RECMID
		call	UpdateLists
endif

		dec	processSnds

		pop	es
		pop	bp
		pop	si
		pop	di
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret
		assume bx:nothing
DoChannelList	ENDP


DoFade		PROC	NEAR		       	;PROCESS NODE FADE PROPERTIES
assume bx:ptr Sound
		push	bx
		push	cx

		cmp	byte ptr es:[bx].sFadeCount,0
		je		ticksOver		      		;If we still have some FadeCount left,
		dec	byte ptr es:[bx].sFadeCount;count it down
		jmp	outDoFade

ticksOver:
		mov	cl,es:[bx].sFadeTicks
		mov	es:[bx].sFadeCount,cl
		mov	cl,es:[bx].sFadeDest	;If we're at the
		and	cl,7fh			;destination volume, stop
		cmp	cl,es:[bx].sVolume	;the fade
		je		endFade
		ja		fadeUp

		mov	cl,es:[bx].sVolume	;Fade the sound down
		mov	ch,es:[bx].sFadeDest
		and	ch,7fh
		sub	cl,ch
		cmp	cl,es:[bx].sFadeSteps
		ja		downSteps
		mov	cl,es:[bx].sFadeDest
		and	cl,7fh
		mov	ch,1
		call	DoChangeVol
		jmp	SHORT endFade
downSteps:
		mov	cl,es:[bx].sVolume
		sub	cl,es:[bx].sFadeSteps
		mov	ch,1
		call	DoChangeVol
		jmp	SHORT outDoFade

fadeUp:
		mov	cl,es:[bx].sFadeDest	;Fade the sound up
		and	cl,7fh
		mov	ch,es:[bx].sVolume
		sub	cl,ch	
		cmp	cl,es:[bx].sFadeSteps
		ja		upSteps
		mov	cl,es:[bx].sFadeDest
		and	cl,7fh
		mov	ch,1
		call	DoChangeVol
		jmp	SHORT endFade
upSteps:
		mov	cl,es:[bx].sVolume
		add	cl,es:[bx].sFadeSteps
		mov	ch,1
		call	DoChangeVol
		jmp	SHORT outDoFade

endFade:
		mov	byte ptr es:[bx].sSignal,0feh	;KLUDGE!!!

		mov	byte ptr es:[bx].sFadeSteps,0
		mov	cl,es:[bx].sFadeDest	      ;Stop the fade and check to see if we
		and	cl,80h		      			;need to end the sound
		cmp	cl,0
		je		outDoFade
		mov	ax,bx
		call	DoEnd
		mov	updateChnls,1

outDoFade:
		pop	cx
		pop	bx
		ret
		assume bx:nothing
DoFade		ENDP


DoChangeVol	PROC	NEAR			;CHANGE SOUND NODE VOLUME
assume bx:ptr Sound
		push	ax
		push	bx
		push	cx
		push	dx
		push	si
		push	di

		mov	vRequest,ch
					
		cmp	cl,es:[bx].sVolume	;If its the same volume as
		jne	newVolNotSame		;before, then don't bother
		jmp	outDoChVol

newVolNotSame:
		mov	es:[bx].sVolume,cl	;Store the new volume

		cmp	si,255			;If the node isn't on the
		jne	volNodeOnList		;PlayList, then exit
		jmp	outDoChVol

volNodeOnList:
		mov	dx,si			;Change the volume of all
		shl	dl,1			;real channels occupied by
		shl	dl,1			;the sound
		xor	si,si			
loopChnlVol:
		mov	cl,ChList[si]					
		cmp	cl,255							
		je		nextChEntry
		mov	ch,cl
		and	cl,0f0h
		cmp	cl,dl
		jne	nextChEntry
		mov	cl,ch
		and	cx,0fh
		mov	di,cx
		mov	cl,es:[bx+di].cVolume
		push	dx
		mov	dl,es:[bx].sVolume
		call	ScaleVolume
		cmp	vRequest,0
		je		sendNewVol
		mov	VolRequest[si],cl
		jmp	SHORT noVolSend
sendNewVol:
		mov	ch,VOLCTRL
		mov	ax,si
		mov	VolRequest[si],255
		CallDriver DController
noVolSend:
		pop	dx
nextChEntry:
		inc	si
		cmp	si,16
		jne	loopChnlVol

		xor	ch,ch			;If there are any ghost
		xor	si,si			;channels in the node, change
loopGhostVols:
		mov	cl,es:[bx+si].tChannel	;their volumes if no one else
		cmp	cl,0ffh			;owns their channels
		je		outDoChVol
		mov	di,cx
		test	es:[bx+di].cFlags,2
		jz		noChGhostVol
		cmp	ChList[di],255
		jne	noChGhostVol
		mov	al,cl
		mov	cl,es:[bx+di].cVolume
		mov	dl,es:[bx].sVolume
		call	ScaleVolume
		cmp	vRequest,0
		je		sendNewGhstVol
		mov	VolRequest[di],cl
		jmp	SHORT noChGhostVol
sendNewGhstVol:
		mov	ch,VOLCTRL
		mov	ax,di
		mov	VolRequest[di],255
		CallDriver DController
noChGhostVol:
		inc	si
		cmp	si,16
		jne	loopGhostVols
					
outDoChVol:
		pop	di
		pop	si
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret
		assume bx:nothing
DoChangeVol	ENDP


DoVolRequests	PROC	NEAR					;PROCESS FADE VOLUME REQUESTS
		xor	bl,bl								;Send the next two (or less)
		mov	al,requestChnl					;volume requests on the
		xor	ah,ah								;VolRequest list
		mov	si,ax
nextVolReq:
		mov	cl,VolRequest[si]
		cmp	cl,255
		je		noRequest
		mov	VolRequest[si],255
		mov	ch,VOLCTRL
		mov	ax,si
		CallDriver DController
		inc	bl
		cmp	bl,2
		je		outVolReq
noRequest:
		inc	si
		cmp	si,16
		jne	notWrapChnl
		xor	si,si
notWrapChnl:
		mov	al,requestChnl
		xor	ah,ah
		cmp	si,ax
		jne	nextVolReq

outVolReq:
		mov	ax,si
		mov	requestChnl,al
		ret
DoVolRequests	ENDP


SoundServer	PROC	FAR						;PARSE EACH NODE ON PLAYLIST
assume bx:ptr Sound
		cmp	processSnds,0
		je		switchOn
		ret

switchOn:
		pushf
		cli
		push	si
		push	di
		push	es
		push	ds
		push	bp

		cmp	updateChnls,0	     ;Update the channel list if
		je	stepNodes						;anything happened in the
		call	DoChannelList	     ;last interrupt to warrant it

stepNodes:
		xor	si,si								;Fade and parse each node
		xor	di,di								;on the PlayList
nextNode2:
		les	bx,PlayList[si]
		mov	ax,es
		or	ax,bx
		je	outServer
		cmp	byte ptr es:[bx].sPause,0
		jne	nextActivate
		cmp	byte ptr es:[bx].sSample,0
		jne	nextActivate
 cmp	byte ptr es:[bx].sPointer,1
 je	nextActivate
		cmp	byte ptr es:[bx].sFadeSteps,0
		je	noFade
		call	DoFade
		cmp	byte ptr es:[bx].sSignal,255
		jne	noFade
		sub	si,4
		jmp	SHORT nextActivate
noFade:
                ;Dac code removed @DAC
   
		call	ParseNode

		cmp	byte ptr es:[bx].sSignal,255
		je	activate
nextActivate:
		add	si,4
activate:
		add	di,4
		cmp	si,(LISTSIZE * 4)
		jne	nextNode2

outServer:
		; Removed DAC code @DAC
		call	DoVolRequests	;volume requests, and
		CallDriver DService	;service the driver

		pop	bp
		pop	ds
		pop	es
		pop	di
		pop	si
		popf
		ret
assume bx:nothing

SoundServer	ENDP



;Removed DAC code - SampleActive - @DAC


;Removed DAC code - DoSamples - @DAC





ParseNode	PROC	NEAR  	        ;PARSE A NODE
assume bx:ptr Sound
		push	si									;node in es:bx
		push	di
		push	bp
		push	ax
		push	bx
		push	cx
		push	dx
		push	ds

ifdef RECMID
		mov	testNode,0
endif

		shl	di,1				;Put the PlayList position
		shl	di,1				;of the node in the 4 hi-
		mov	cx,di				;bits of playPos
		mov	playPos,cl

		inc	word ptr es:[bx].sTimer		;Increment the timer


ifdef RECMID		      				;If the resource
		lds	bp,dword ptr es:[bx].sPointer  
		mov	ax,ds			      	;pointer is ffff:ffff,
		cmp	ax,0ffffh	       	;then it's a test node,
		je	maybeTest	      	;and will be receiving
		jmp	SHORT regularParse	;data directly from the
maybeTest:
		cmp	bp,0ffffh		;MPU
		je	irregParse
		jmp	SHORT regularParse
irregParse:
		call	DoTestNode
		cmp	si,0
		jne	notSi0
		jmp	endParse
notSi0:
		xor	si,si
		mov	dx,cs
		mov	ds,dx
		mov	bp,offset MIDIbuffer
		jmp	parseCommand
endif

regularParse:
		Hnd2Ptr	es:[bx].sPointer
		mov	baseIndex,bp		;pointer

		xor	si,si			;Step through tracks
loopParseTrk:
		mov	al,es:[bx+si].tChannel	;until a channel# of 0ffh
		cmp	al,0ffh			;is encountered.  Skip any
		jne	notLastTrk		;sample tracks, marked with
		jmp	outParse		;0feh
notLastTrk:
		cmp	al,0feh
		jne	goodTrack
		jmp	parseNext

goodTrack:
		mov	realChnl,255		;Check to see if the current
		mov	ghostChnl,0		;node-channel is on the ChList,
		push	si			;or if the channel is a ghost
		mov	si,ax			;channel
		and	si,0ffh
		test	es:[bx+si].cFlags,2
		pop	si
		jz		notGhostChnl
		mov	realChnl,al
		mov	ghostChnl,1
		jmp	SHORT restorePtr
notGhostChnl:
		and	al,0fh
		mov	cl,al			
		or		cl,playPos		
		xor	di,di
lookForCh:
		cmp	ChList[di],cl
		je		chOnList
		inc	di
		cmp	di,16
		jne	lookForCh
		jmp	SHORT restorePtr
chOnList:
		mov	dx,di
		mov	realChnl,dl

restorePtr:
		mov	bp,baseIndex			;Point ds:bp to the location
		shl	si,1			        ;in the track data where we
		mov	dx,ds:[bp+si]			;last left off
		add	bp,dx
		add	bp,es:[bx+si].tIndex
		cmp	word ptr es:[bx+si].tIndex,0
		jne	notFrozenTrk
		shr	si,1
		jmp	parseNext
notFrozenTrk:
		shr	si,1

		shl	si,1				;Check for a rest
		cmp	word ptr es:[bx+si].tRest,0     ;count.  If there is,
		je	restOver			;decrement it and
		dec	word ptr es:[bx+si].tRest	;move on to the next
		cmp	word ptr es:[bx+si].tRest,8000h	;track
		jne	notDoneOver
		xor	dh,dh
		shr	si,1
		getb	dl
		shl	si,1
		cmp	dl,TIMINGOVER
		jne	notTOAgain
		mov	dl,240
		mov	dh,80h
notTOAgain:
		mov	es:[bx+si].tRest,dx
notDoneOver:
		shr	si,1
		jmp	parseNext						
restOver:
		shr	si,1

parseCommand:
		getb	dl		        ;Read next byte in MIDI
		cmp	dl,128			;stream, and determine
		jb	runningStat		;if it's running
		mov	es:[bx+si].tCommand,dl	;status or not
		jmp	SHORT parseIt
runningStat:
		mov	dl,es:[bx+si].tCommand
		dec	bp
		shl	si,1
		dec	word ptr es:[bx+si].tIndex
		shr	si,1

parseIt:
		mov	al,dl			;Split command and channel
		mov	ah,al
		and	ah,0f0h
		and	al,0fh

ifdef RECMID					;If the node is a test node,
		cmp	testNode,0		;set the realChnl to what
		je	notAutoChnl	        ;the command byte said
		mov	realChnl,al
endif

notAutoChnl:                                    
		cmp	dl,ENDTRK		;If the end of the track
		jne	notEndTrk		;has been reached, mark
		shl	si,1			;this track as being
		mov	word ptr es:[bx+si].tIndex,0 ;done, and move on to
		shr	si,1			;the next track
		jmp	parseNext							

notEndTrk:
		cmp	al,15			;If we're on channel 16,
		jne	notControlCh		;parse with the
		call	ControlChnl	     	;ControlCh procedure.
		shl	si,1			;If we return with a 0
		mov	dx,es:[bx+si].tIndex	;index, then we hit an
		shr	si,1			;end point
		cmp	dx,0
		jne	nextComm
		jmp	parseNext

notControlCh:
		mov	al,realChnl		;Get the real Channel #, then
		cmp	ah,NOTEOFF		;Switch on the value of the
		jne	notNoteOff		;command in ah to any of
		call	NoteOff			;the eight MIDI handling
		jmp	SHORT nextComm		;procedures
notNoteOff:
		cmp	ah,NOTEON
		jne	notNoteOn
		call	NoteOn
		jmp	SHORT nextComm
notNoteOn:
		cmp	ah,POLYAFTER
		jne	notPolyAfter
		call	PolyAfterTch
		jmp	SHORT nextComm
notPolyAfter:
		cmp	ah,CONTROLLER
		jne	notContrlr
		call	Controller
		jmp	SHORT nextComm
notContrlr:
		cmp	ah,PCHANGE
		jne	notPChange
		call	ProgramChange
		jmp	SHORT nextComm
notPChange:
		cmp	ah,CHNLAFTER
		jne	notChnlAfter
		call	ChnlAfterTch
		jmp	SHORT nextComm
notChnlAfter:
		cmp	ah,PBEND
		jne	notPBend
		call	PitchBend
		jmp	SHORT nextComm
notPBend:
		cmp	ah,SYSEX
		jne	notSysex
		call	SysEx
		jmp	SHORT nextComm

notSysex:
		shl	si,1			        ;If the command wasn't
		mov	word ptr es:[bx+si].tIndex,0    ;any of the above, then
		shr	si,1			        ;shut down the track
		jmp	SHORT parseNext		        ;before we do any
						     						;serious damage.

nextComm:
		getb	dl									;Check for timing
		cmp	dl,0
		jne	endOfParse
		jmp	parseCommand

endOfParse:
ifdef RECMID						;If the node was a test node,
		cmp	testNode,0			;then exit
		je	notTestNode		
		jmp	endParse
endif

notTestNode:
		shl	si,1								;Put the timing byte in
		cmp	dl,TIMINGOVER					;the tracks tRest counter.
		jne	regularTime						;Subtract 1 because it will
		mov	dl,239							;take one tick before the
		mov	dh,80h					;next timing check
		mov	es:[bx+si].tRest,dx
		shr	si,1
		jmp	SHORT parseNext
regularTime:
		xor	dh,dh
		dec	dl
		mov	es:[bx+si].tRest,dx
		shr	si,1

parseNext:
		inc	si									;Loop to next track
		cmp	si,16
		je		outParse
		jmp	loopParseTrk

outParse:
		xor	si,si				 				;If the sound is
checkForEnd:
		cmp	byte ptr es:[bx+si].tChannel,255 ;over, either loop
		je		allFrozen		 				;it or end it
		shl	si,1
		cmp	word ptr es:[bx+si].tIndex,0
		jne	endParse
		shr	si,1
nextEndChk:
		inc	si
		cmp	si,16
		jne	checkForEnd
allFrozen:
		cmp	byte ptr es:[bx].sHold,0
		jne	loopSound
		cmp	byte ptr es:[bx].sLoop,0
		jne	loopSound
findEndSpot:
		mov	ax,bx
		call	DoEnd
		mov	updateChnls,1
		jmp	SHORT endParse
loopSound:
		mov	dx,es:[bx].sLoopTime
		mov	es:[bx].sTimer,dx
		xor	si,si
loopLoopSnd:
		mov	dx,es:[bx+si].tLoopPoint
		mov	es:[bx+si].tIndex,dx
		mov	dx,es:[bx+si].tLoopRest
		mov	es:[bx+si].tRest,dx
		shr	si,1
		mov	dl,es:[bx+si].tLoopCommand
		mov	es:[bx+si].tCommand,dl
		shl	si,1
		add	si,2
		cmp	si,32
		jne	loopLoopSnd

endParse:
		pop	ds
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		pop	bp
		pop	di
		pop	si
		ret
		assume bx:nothing
ParseNode	ENDP


NoteOff		PROC	NEAR						;TURN NOTE OFF
assume bx:ptr Sound
		push	si

		getb	ch									;Put note # in ch, and its
		getb	cl									;velocity in cl

		push	ax									;Get node channel index
		mov	al,es:[bx+si].tChannel
		mov	si,ax
		and	si,000000000fh
		pop	ax

;;;		cmp	al,16
;;;		jae	notCurNoteOff
		cmp	es:[bx+si].cCurNote,ch
		jne	notCurNoteOff
		mov	byte ptr es:[bx+si].cCurNote,255
notCurNoteOff:
		cmp	al,255
		je		outNoteOff
		cmp	restoring,0
		jne	outNoteOff
		and	al,0fh
		CallDriver DNoteOff

outNoteOff:
		pop	si
		ret
		assume bx:nothing
NoteOff		ENDP


NoteOn		PROC	NEAR						;TURN NOTE ON
assume bx:ptr Sound
		push	si

		getb	ch									;Put note # in ch, and its
		getb	cl									;velocity in cl

		push	ax									;Get node channel index
		mov	al,es:[bx+si].tChannel
		mov	si,ax
		and	si,000000000fh
		pop	ax

		cmp	cl,0								;Update the cCurNote property
		je		doNoteOff						;of the sound node
;;;		cmp	al,16
;;;		jae	dontSaveNote
		mov	es:[bx+si].cCurNote,ch
dontSaveNote:
		cmp	al,255
		je		outNoteOn
		cmp	restoring,0
		jne	outNoteOn
		and	al,0fh
		CallDriver DNoteOn
		jmp	SHORT outNoteOn
doNoteOff:
;;;		cmp	al,16
;;;		jae	notCurNote
		cmp	es:[bx+si].cCurNote,ch
		jne	notCurNote
		mov	byte ptr es:[bx+si].cCurNote,255
notCurNote:
		cmp	al,255
		je		outNoteOn
		cmp	restoring,0
		jne	outNoteOn
		and	al,0fh
		CallDriver DNoteOff

outNoteOn:
		pop	si
		ret
		assume bx:nothing
NoteOn		ENDP


PolyAfterTch	PROC	NEAR					;DO POLY AFTERTOUCH
		getb	ch									;Put note # in ch and
		getb	cl									;pressure in cl

		cmp	al,255
		je		noPolyAfter
		cmp	restoring,0
		jne	noPolyAfter
		CallDriver DPolyAfterTch
noPolyAfter:
		ret
PolyAfterTch	ENDP


Controller	PROC	NEAR						;DO CONTROL CHANGE
assume bx:ptr Sound
		push	si

		getb	ch									;Put controller # in ch and
		getb	cl									;value in cl

		test	ghostChnl,0ffh
		jz		notGhostCont
		push	si
		mov	si,ax
		and	si,000000000fh
		cmp	ChList[si],255
		pop	si
		je		notGhostCont
		jmp	outContrlr

notGhostCont:
		push	ax									;Get node channel index
		mov	al,es:[bx+si].tChannel
		mov	si,ax
		and	si,000000000fh
		pop	ax

		cmp	ch,VOLCTRL						;Save applicable controller
		jne	notVolCtrl						;values in sound node
		mov	es:[bx+si].cVolume,cl
		mov	dl,es:[bx].sVolume
		call	ScaleVolume
		cmp	al,32
		jb		clrVolRequest
		jmp	outContrlr
clrVolRequest:
		push	si
		mov	si,ax
		and	si,0ffh
		mov	VolRequest[si],255
		pop	si
		jmp	SHORT doCtrlr
notVolCtrl:
		cmp	ch,PANCTRL
		jne	notPanCtrl
		mov	es:[bx+si].cPan,cl
		jmp	SHORT doCtrlr
notPanCtrl:
		cmp	ch,MODCTRL
		jne	notModCtrl
		mov	es:[bx+si].cModulation,cl
		jmp	SHORT doCtrlr
notModCtrl:
		cmp	ch,DAMPRCTRL
		jne	notDamprCtrl
		push	dx
		shl	si,1
		mov	dx,es:[bx+si].cDamprPbend
		or		dh,80h
		cmp	cl,0
		jne	notOffDampr
		and	dh,7fh
notOffDampr:
		mov	es:[bx+si].cDamprPbend,dx
		shr	si,1
		pop	dx
		jmp	SHORT doCtrlr
notDamprCtrl:
		cmp	ch,NUMNOTES
		jne	notNumNotes
		push	cx
		mov	ch,es:[bx+si].cPriVoice
		and	ch,0f0h
                and     cl,0fh             ;Enforce NUMNOTES 0-15, @NUM
		or	ch,cl
		mov	es:[bx+si].cPriVoice,ch
		pop	cx
		mov	updateChnls,1
		jmp	SHORT doCtrlr
notNumNotes:
		cmp	ch,MUTECTRL
		jne	doCtrlr
		push	cx
		mov	ch,es:[bx+si].cMute
		and	ch,0f0h
		test	cl,0ffh
		je		muteOff
		mov	cl,1
muteOff:
		or		ch,cl
		mov	es:[bx+si].cMute,ch
		pop	cx
		mov	updateChnls,1

doCtrlr:
		cmp	al,255
		jae	outContrlr
		cmp	restoring,0
		jne	outContrlr
		and	al,0fh
		CallDriver DController

outContrlr:
		pop	si
		ret
		assume bx:nothing
Controller	ENDP


ProgramChange	PROC	NEAR					;DO PROGRAM CHANGE
assume bx:ptr Sound
		push	si

		getb	cl									;Put program # in cl

		test	ghostChnl,0ffh
		jz		notGhostPch
		push	si
		mov	si,ax
		and	si,000000000fh
		cmp	ChList[si],255
		pop	si
		je		notGhostPch
		jmp	SHORT noPChange

notGhostPch:
		push	ax						;Get node channel index
		mov	al,es:[bx+si].tChannel
		mov	si,ax
		and	si,000000000fh
		pop	ax

		mov	es:[bx+si].cProgram,cl		;Save program in sound node

		cmp	al,255
		jae	noPChange
		cmp	restoring,0
		jne	noPChange
		and	al,0fh
		CallDriver DProgramChange

noPChange:
		pop	si
		ret
		assume bx:nothing
ProgramChange	ENDP


ChnlAfterTch	PROC	NEAR					;DO CHANNEL AFTERTOUCH
		getb	cl									;Put pressure in cl

		cmp	al,255
		jae	noChnlAfter
		cmp	restoring,0
		jne	noChnlAfter
		CallDriver DChnlAfterTch
noChnlAfter:
		ret
ChnlAfterTch	ENDP


PitchBend	PROC	NEAR						;DO PITCH BEND
assume bx:ptr Sound
		push	si

		getb	ch									;Put lsb in ch and msb in cl
		getb	cl

		test	ghostChnl,0ffh
		jz		notGhostPbend
		push	si
		mov	si,ax
		and	si,000000000fh
		cmp	ChList[si],255
		pop	si
		je		notGhostPbend
		jmp	SHORT noPBend

notGhostPbend:
		push	ax						;Get node channel index
		mov	al,es:[bx+si].tChannel
		mov	si,ax
		and	si,000000000fh
		pop	ax

		push	cx
		xchg	ch,cl
		shr	ch,1
		jnc	noHiPitchBit2
		or		cl,80h
noHiPitchBit2:
		shl	si,1
		cmp	word ptr es:[bx+si].cDamprPbend,8000h
		jb		noDamper
		or		ch,80h
noDamper:
		mov	es:[bx+si].cDamprPbend,cx
		shr	si,1
		pop	cx

		cmp	al,255
		jae	noPBend
		cmp	restoring,0
		jne	noPBend
		and	al,0fh
		CallDriver DPitchBend

noPBend:
		pop	si
		ret
		assume bx:nothing
PitchBend	ENDP


SysEx		PROC	NEAR							;PROCESS SYSEX
		call	SkipMidi							;We don't support it
		ret
SysEx		ENDP


ControlChnl	PROC	NEAR						;CONTROL CHANNEL COMMANDS
assume bx:ptr Sound
		cmp	ah,PCHANGE						;If it's not a valid Ch. 16
		je		doCue								;command, then skip it
		cmp	ah,CONTROLLER
		jne	notControlComm
		jmp	doContrlr
notControlComm:
		call	SkipMidi
		jmp	outConCh

doCue:
			getb	ch								;If it's a loop set, set the
		cmp	ch,127							;loop point.  If not, set
		jne	notLoopCue						;the signal property of the
		push	dx									;sound node
		getb	dl
		xor	dh,dh
		cmp	dl,TIMINGOVER
		jne	notLoopTOver
		mov	dh,80h
		mov	dl,240
notLoopTOver:
		shl	si,1
		mov	es:[bx+si].tRest,dx
		shr	si,1
		pop	dx
		mov	byte ptr es:[bx+si].tCommand,0cfh
		push	si
		push	dx
		xor	si,si
loopLoopPoint:
		shl	si,1
		mov	dx,es:[bx+si].tIndex
		mov	es:[bx+si].tLoopPoint,dx
		mov	dx,es:[bx+si].tRest
		mov	es:[bx+si].tLoopRest,dx
		shr	si,1
		mov	dl,es:[bx+si].tCommand
		mov	es:[bx+si].tLoopCommand,dl
		inc	si
		cmp	si,16
		jne	loopLoopPoint
		mov	dx,es:[bx].sTimer
		mov	es:[bx].sLoopTime,dx
		pop	dx
		pop	si
		shl	si,1
		dec	word ptr es:[bx+si].tIndex
		dec	bp
		mov	word ptr es:[bx+si].tRest,0
		shr	si,1
		jmp	SHORT outConCh
notLoopCue:
		cmp	restoring,0
		jne	outConCh
		mov	es:[bx].sSignal,ch
		jmp	SHORT outConCh

doContrlr:
		getb	ch									;If it's reverb, set a new
		getb	cl									;ReverbMode.  If it's a
		cmp	ch,REVERBMODE					;DataInc cue, increment the
		jne	notReverb						;value in the node.  If it's
		cmp	cl,07fh							;an end point, and sHold
		jne	notNewRevDef					;matches the value, loop to
		mov	cl,revDefault					;last loop point
notNewRevDef:
		mov	es:[bx].sReverbMode,cl
		push	ax
		CallDriver DSetReverb
		pop	ax
		jmp	SHORT outConCh
notReverb:
		cmp	ch,DATAINC
		jne	notDataInc
		cmp	restoring,0
		jne	outConCh
		inc	word ptr es:[bx].sDataInc
		jmp	SHORT outConCh
notDataInc:
		cmp	ch,ENDPOINT
		jne	outConCh
		
		cmp	es:[bx].sHold,cl
		jne	outConCh
		push	si
		xor	si,si
loopEndPoint:
		mov	word ptr es:[bx+si].tIndex,0
		add	si,2
		cmp	si,32
		jne	loopEndPoint
		pop	si

outConCh:
		ret
		assume bx:nothing
ControlChnl	ENDP


SkipMidi	PROC	NEAR							;SKIP NEXT MIDI COMMAND
		cmp	ah,SYSEX
		jne	notSYSEX
keepSkipping:
		getb	ch
		cmp	ch,EOX
		jne	keepSkipping
		ret

notSYSEX:
		cmp	ah,PCHANGE
		je		skipOnly1
		cmp	ah,CHNLAFTER
		je		skipOnly1
		getb	ch
skipOnly1:
		getb	ch
		ret
SkipMidi	ENDP


ScaleVolume	PROC	NEAR						;SCALE VOLUME CONTROLLER
		push	ax									;
		mov	al,cl								; cVol	 x
		xor	ah,ah								; ---- = ----
		inc	al									; 127	 sVol
		inc	dl
		mul	dl
		shl	ah,1
		mov	cl,ah
		cmp	cl,0
		je		outScale
		dec	cl
outScale:
		pop	ax
		ret
ScaleVolume	ENDP


FindNode	PROC	NEAR							;FIND A NODE ON THE PLAYLIST
		push	cx

		mov	cx,es			 					;Locate node es:bx on the
		xor	si,si			 					;PlayList and return the
lookForNode:
		cmp	word ptr PlayList[si],bx ;index in si
		jne	notFoundNode
		cmp	word ptr PlayList[si+2],cx
		jne	notFoundNode
		jmp	SHORT outFnode
notFoundNode:
		add	si,4
		cmp	si,(LISTSIZE * 4)
		jne	lookForNode
		mov	si,255

outFnode:
		pop	cx
		ret
FindNode	ENDP


FixupHeader	PROC	NEAR						;CREATE SOUND RESOURCE HEADER
assume bx:ptr Sound
		push	bx
		push	cx
		push	dx
		push	bp
		push	ds

		mov	bx,ax			         		;Get Resource pointer
		cmp	word ptr es:[bx].sPointer,0ffffh
		jne	notReceiveNode
		cmp	word ptr es:[bx+2 ].sPointer,0ffffh
		jne	notReceiveNode
		jmp	outFixup
notReceiveNode:
		Hnd2Ptr	es:[bx].sPointer

		cmp	byte ptr ds:[bp+35],0feh 	;Make sure that this
		jne	doHeader		 					;resource hasn't already
		cmp	byte ptr ds:[bp+34],0fdh 	;been fixed up
		jne	doHeader
		cmp	byte ptr ds:[bp+33],0fch
		jne	doHeader
		jmp	outFixup

doHeader:
		push	bp									;Clear header buffer
		mov	si,32
clearHeader:
		sub	si,2
		mov	KludgeHdr[si],0
		cmp	si,0
		jne	clearHeader

		mov	priInfo,-1						;If there is a properties
		cmp	byte ptr ds:[bp],0f0h		;entry in the header,
		jne	checkDevID						;pull the properties
		mov	cl,ds:[bp+1]					;information out of it
		mov	priInfo,cl
		add	bp,8

checkDevID:
		mov	cl,ds:[bp]						;Find device ID in header
		cmp	cl,devID
		je		gotDevice
		cmp	cl,0ffh
		je		writeHeader
		inc	bp
findEndDev:
		mov	cl,ds:[bp]
		inc	bp
		cmp	cl,0ffh
		jne	notEndDev
		jmp	SHORT checkDevID
notEndDev:
		add	bp,5
		jmp	SHORT findEndDev

gotDevice:
		inc	bp									;Construct the Kludged
findDevTracks:
		mov	cl,ds:[bp]						;header
		inc	bp
		cmp	cl,0ffh
		je		writeHeader
		inc	bp
		mov	cx,ds:[bp]
		add	bp,4
		mov	KludgeHdr[si],cx
		add	si,2
		jmp	SHORT findDevTracks

writeHeader:
		pop	bp									;Write the header onto the
		push	bp									;sound resource
		xor	si,si
writeMoreHdr:
		mov	cx,KludgeHdr[si]
		mov	ds:[bp],cx
		add	si,2
		add	bp,2
		cmp	si,32
		jne	writeMoreHdr

		mov	cl,priInfo						;Put the priority information
		mov	ds:[bp],cl						;into the new header

		pop	bp			 						;Flag this resource as
		mov	byte ptr ds:[bp+33],0fch	;an already "fixed-up"
		mov	byte ptr ds:[bp+34],0fdh	;resource
		mov	byte ptr ds:[bp+35],0feh

outFixup:
		pop	ds
		pop	bp
		pop	dx
		pop	cx
		pop	bx
		ret
		assume bx:nothing
FixupHeader	ENDP


ifdef RECMID

AskDriver	PROC	FAR						;GET STATE FROM DRIVER
		pushf
		cli

		cmp	ah,0
		jne	notAskMVol
		mov	cl,255
		CallDriver DMasterVol
		jmp	SHORT outAsk
notAskMVol:
		cmp	ah,1
		jne	notAskRev
		mov	cl,255
		CallDriver DSetReverb
		jmp	SHORT outAsk
notAskRev:
		cmp	ah,2
		jne	notAskSndOn
		mov	cl,255
		CallDriver DSoundOn
		jmp	SHORT outAsk

notAskSndOn:
		CallDriver DAskDriver

outAsk:
			popf
		ret
AskDriver	ENDP



;---------------------------------------------------------------------------
;---------------------------------------------------------------------------
;       Midi Receiving functions (for MTEST)


;**********************************************************
;* Get MIDI byte from MIDI receive interrupt buffer - @INT
;**********************************************************


MIDIget		PROC	NEAR
                push    bx

                xor     ax,ax
receiveNext:
                .if MIDIbcount == 0      ;If no bytes return 255 in al
                        mov     al,255
                        jmp     outMIDIget
                .endif

                dec     MIDIbcount              ;Decrement byte count
                mov     bx,MIDIbhead            ;Buffer empty pointer
                mov     al,cs:[bx]              ;Get character from buffer
                inc     bx                      ;Bump buffer empty pointer

                cmp     bx,offset cs:MIDIbufend ;Test for end of buffer
                jb      NotAtEnd                ;no, continue
                mov     bx,offset cs:MIDIbufbeg ;Yes, set to beginning
NotAtEnd:
                mov     MIDIbhead,bx            ;Update empty buff pointer

		cmp	al,0f8h                 ;Timing clock, skip it
		je	receiveNext
		cmp	al,0feh                 ;Active sensing, skip it
		je	receiveNext
		cmp	al,0fch                 ;Stop message, skip it
		je	receiveNext      
		cmp	al,0fah
		jb	outMIDIget              ;Got a byte, exit
		call	SendReset
		jmp	receiveNext

outMIDIget:
                pop     bx
		ret
MIDIget		ENDP


;**************************************************
;* Put 1 byte back into MIDI receive buffer - @INT 
;**************************************************

MIDIback1       PROC    FAR
                push    bx


                inc     MIDIbcount              ;Increment byte count
                mov     bx,MIDIbhead            ;Buffer empty pointer
                dec     bx                      ;Dec buffer empty pointer

                cmp     bx,offset cs:MIDIbufbeg ;Test for beg. of buffer
                jae     NotAtBeg                ;no, continue
                mov     bx,offset cs:MIDIbufend ;Yes, set to end
                dec     bx
NotAtBeg:
                mov     MIDIbhead,bx            ;Update empty buff pointer


                pop     bx
		ret
MIDIback1       ENDP



;***********************************************************
;* Stash MIDI bytes received in driver during output - @INT
;***********************************************************


StashMIDI       PROC    FAR
                push    bx
		pushf
		cli

                cmp     install,1
                jne     EndStash

                mov     bx,MIDIbtail    ;Buffer fill pointer
                mov     cs:[bx],al      ;Store character in buffer
                inc     bx              ;Bump pointer

                cmp     bx,offset cs:MIDIbufend ;Test for buffer end
                jb      NotEnd                  ;No, continue
                mov     bx,offset cs:MIDIbufbeg ;offset of buffer start
NotEnd:         
                mov     MIDIbtail,bx    ;Update buffer fill pointer
                inc     MIDIbcount      ;Increment count of bytes

EndStash:
                popf
                pop     bx
                ret
StashMIDI       ENDP


;****************************************
;* MIDI receive interrupt handler - @INT
;****************************************

                PUBLIC MPU_int

MPU_int         PROC    FAR

                push    ds
                push    es
                push    bp
                push    di
                push    si
                push    dx
                push    cx
                push    bx
                push    ax
                
                ;Test to see if MIDI interrupt

                mov     dx,STATPORT     ;Status port
                in      al,dx           ;Read status
                test    al,DSR          ;Data ready to receive?         
                jz      BytesWaiting    ;yes, get it

                ;Pass interrupt on

                pop     ax
                pop     bx
                pop     cx
                pop     dx
                pop     si
                pop     di
                pop     bp
                pop     es
                pop     ds
                jmp     dword ptr SavIntPtr


BytesWaiting:
                ;cmp     MIDIbcount,MIDIBSIZE-1
                ;jge     OutInt

                mov     dx,DATAPORT     ;Data port
                in      al,dx           ;Read byte

                mov     bx,MIDIbtail    ;Buffer fill pointer
                mov     cs:[bx],al      ;Store character in buffer
                inc     bx              ;Bump pointer

                cmp     bx,offset cs:MIDIbufend ;Test for buffer end
                jb      NotEnd                  ;No, continue
                mov     bx,offset cs:MIDIbufbeg ;offset of buffer start
NotEnd:         
                mov     MIDIbtail,bx    ;Update buffer fill pointer
                inc     MIDIbcount      ;Increment count of bytes
OutInt:
                mov     al,00100000b    ;EOI for 8259
                out     piccmd,al       ;Reset 8259
     
                pop     ax
                pop     bx
                pop     cx
                pop     dx
                pop     si
                pop     di
                pop     bp
                pop     es
                pop     ds
                
                iret
MPU_int         ENDP



;************************************
;* Set MIDI receive interrupt - @INT
;************************************

Set_M_Interrupt PROC    NEAR
                push    ax
                push    bx      
                push    cx
                push    dx

                ;Initialize buffer pointers

                mov     MIDIbcount,0           ;Set count to zero  
                mov     ax,offset cs:MIDIbufbeg
                mov     MIDIbhead,ax           ;Empty pointer
                mov     MIDIbtail,ax           ;Fill pointer

                ;Check if already installed

                cmp     install,1
                je      EndSetM
                
                mov     install,1              ;Set install flag
                
                ;generate pic mask - mask = (1 << intlevel) ^ 0xFF

                mov     cl,2
                mov     al,1
                shl     al,cl
                not     al
                mov     imask,al

                ;Get the current interrupt handler

                push    es
                mov     ah,35h          ;Get vector
                mov     al,midi_level   ;midi port interrupt level
                int     21h             ;Get current vector
                mov     SavIntPtr,bx    ;Save old vector offset
                mov     SavIntPtrS,es   ;Save old vector segment
                pop     es

                ;Establish new handler

                push    ds
                mov     dx,offset cs:MPU_int
                mov     ah,25h
                mov     al,midi_level
                push    cs
                pop     ds
                int     21h
                pop     ds

                ;Enable interrupt

                in      al,picmsk       ;read 8259 interrupt mask
                and     al,imask        ;enable new interrupt
                out     picmsk,al       ;output new mask

EndSetM: 
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                ret
Set_M_Interrupt ENDP



;*******************************************
;* Disconnect MIDI receive interrupt - @INT
;*******************************************

Drop_M_Interrupt PROC    NEAR
                push    ax
                push    bx        

                .if install == 0
                        jmp     OutDrop
                .endif

                mov     install,0              ;Reset install flag

                ;Disable interrupt

                cli
                in      al,picmsk       ;Get 8259 interrupt mask
                mov     bl,imask        ;Get interrupt mask byte
                not     bl              ;Invert it
                or      al,bl           ;Turn off interrupt
                out     picmsk,al       ;Output mask
                sti

                ;Restore original interrupt vector
                
                push    ds
                mov     ah,25h
                mov     al,midi_level                   
                mov     dx,SavIntPtr
                mov     ds,SavIntPtrS
                int     21h
                pop     ds
                
OutDrop:       
                pop     bx
                pop     ax
                ret
Drop_M_Interrupt ENDP







MPUcomm		PROC	NEAR	   	;SEND COMMAND BYTE TO MPU
		push	ax
		push	cx
		push	dx
		mov	cx,65535

		mov	dx,STATPORT

waitForMPU:
		in	al,dx									;Wait for hardware DRR
		test	al,DRR
		jz		sendComm
		dec	cx
		cmp	cx,0
		jne	waitForMPU
		jmp	SHORT outMTComm

sendComm:
		mov	al,bl								;Send command to MPU
		out	dx,al
		mov	cx,65535

waitForMPU2:
		in	al,dx
		rol	al,1
		jnb	data2Res
		dec	cx
		cmp	cx,0
		jne	waitForMPU2

data2Res:
		mov	dx,DATAPORT

		in	al,dx									;Signal an error if
		cmp	al,ACK							;there is no response
		je		outMTComm

outMTComm:
		pop	dx
		pop	cx
		pop	ax
		ret
MPUcomm		ENDP




ResetMPU	PROC	NEAR
		pushf
		cli
		push	ax
		push	bx
		push	cx
		push	dx

tryResAgain:
		mov	cx,65535
		mov	bl,0ffh
		mov	dx,STATPORT

waitForMPUres:
		in	al,dx									;Wait for hardware DRR
		test	al,DRR
		jz	sendResComm
		dec	cx
		cmp	cx,0
		jne	waitForMPUres
		jmp	SHORT outMTResComm

sendResComm:
		mov	al,bl								;Send command to MPU
		out	dx,al
		mov	cx,65535

waitForMPU2res:
		in	al,dx
		rol	al,1
		jnb	data2Res
		dec	cx
		cmp	cx,0
		jne	waitForMPU2res

data2Res:
		mov	dx,DATAPORT

		in	al,dx									;Signal an error if
		cmp	al,ACK							;there is no response
		je	outMTResComm
		jmp	SHORT tryResAgain

outMTResComm:
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		popf
		ret
ResetMPU	ENDP





DoTestNode	PROC	NEAR	   ;FILL MIDI RECEIVE BUFFER
		push	bx
		push	es
                pushf
                cli

;		mov	si,136
;loopClrTest:
;		dec	si
;		mov	MIDIbuffer[si],255
;		cmp	si,0
;		jne	loopClrTest

		mov	testNode,1
		xor	si,si
moreMidiBuff:
		;cmp	si,135
		;jbe	notFullMid
		;call	ClearMidInfo
		;jmp	SHORT outDoTest
;notFullMid:
		call	MIDIget
		cmp	al,255
		je	outDoTest
		cmp	al,0f0h
		jne	notSysEx
		call	SkipSysEx
		jmp	SHORT moreMidiBuff
notSysEx:
		cmp	al,128
		jae	noBuffRunn
                mov     RunningStat,1
		mov	MIDIbuffer[si+1],al
		mov	al,oldRecCom
		mov	MIDIbuffer[si],al
		add	si,2
		jmp	SHORT putInBuff
noBuffRunn:
                mov     RunningStat,0
		mov	oldRecCom,al
		mov	MIDIbuffer[si],al
		inc	si
		call	MIDIget      
                cmp     al,255       ;If complete command
                jne     Gotit1       ;is not ready 
                call    MIDIback1    ;Put byte back in buffer @INT
                dec     si
                JMP     SHORT outDoTest
Gotit1:
		mov	MIDIbuffer[si],al
		inc	si
putInBuff:
		mov	bl,oldRecCom
		and	bl,0f0h
		cmp	bl,CHNLAFTER
		je	oneBuffByte
		cmp	bl,PCHANGE
		je	oneBuffByte
		call	MIDIget
                cmp     al,255       ;If complete command
                jne     Gotit2       ;is not ready 
                call    MIDIback1    ;Put 2 bytes back in buffer @INT
                .if !RunningStat     ;or 1 byte if running status
                  call    MIDIback1    
                  dec     si
                .endif
                dec     si
                JMP     SHORT outDoTest
Gotit2:
		mov	MIDIbuffer[si],al
		inc	si
oneBuffByte:
		mov	MIDIbuffer[si],0
		inc	si
		jmp	moreMidiBuff

outDoTest:
		cmp	si,0
		je	noEndBuff
		dec	si
		mov	MIDIbuffer[si],1

noEndBuff:
                popf
		pop	es	
		pop	bx
		ret					
DoTestNode	ENDP




;ClearMidInfo	PROC	NEAR
;notEmptyYet:
;		call	MIDIget
;		cmp	al,255
;		jne	notEmptyYet
;		ret
;ClearMidInfo	ENDP


SkipSysEx	PROC	NEAR
loopSkipSysEx:
		call	MIDIget
		cmp	al,0f7h
		jne	loopSkipSysEx
		ret
SkipSysEx	ENDP


SendReset	PROC	NEAR
		ret
		push	ax
		push	cx

		xor	al,al
loopSendReset:
		mov	ch,NUMNOTES
		xor	cl,cl
		CallDriver DController
		inc	al
		cmp	al,16
		jne	loopSendReset

		pop	cx
		pop	ax
		ret
SendReset	ENDP


endif

		END
