ADLIB_I equ 1

;     ADLIB.I
;     Definitions of sound chip parameters.
;     1987/01/20, Marc Savary, Ad Lib Inc.
;     Converted to assembly language, 7/88, Stuart Goldstein, Sierra On-Line

;Parameters of each voice:
nbLocParam   EQU    14

prmKsl       EQU    0
prmMulti     EQU    1
prmFeedBack  EQU    2              ; use for opr. 0 only
prmAttack    EQU    3
prmSustain   EQU    4
prmStaining  EQU    5              ; Sustaining ...
prmDecay     EQU    6
prmRelease   EQU    7
prmLevel     EQU    8
prmAm        EQU    9
prmVib       EQU    10
prmKsr       EQU    11
prmFm        EQU    12             ; use for opr. 0 only
prmWaveSel   EQU    13             ; wave select

;globals parameters:
prmAmDepth   EQU    14
prmVibDepth  EQU    15
prmNoteSel   EQU    16
prmPercussion EQU   17

;melodic voice numbers:
vMelo0       EQU    0
vMelo1       EQU    1
vMelo2       EQU    2
vMelo3       EQU    3
vMelo4       EQU    4
vMelo5       EQU    5
vMelo6       EQU    6
vMelo7       EQU    7
vMelo8       EQU    8

;percussive voice numbers:
BD           EQU    6
SD           EQU    7
TOM          EQU    8
CYMB         EQU    9
HIHAT        EQU    10


MAX_VOLUME   EQU    7fh
MAX_PITCH    EQU    3fffh
MID_PITCH    EQU    2000h

MID_C        EQU    60             ; MIDI standard mid C
CHIP_MID_C   EQU    48             ; sound chip mid C
NR_NOTES     EQU    96             ; # of notes we can play on chip

