;-------------------------------------------------------------------
;	errmsg.i
;
;	Defines for ErrMsg
;-------------------------------------------------------------------

ifndef	ERRMSG_I

ERRMSG_I		equ	1


E_DISK_ERROR		equ	1	; Messages 1-24 pre-loaded into buffer
E_CANCEL				equ	2
E_QUIT				equ	3
E_DISK_ERROR_MSG	equ	4	; Disk error messages 4-16 
E_OOPS_TXT			equ	25
E_OOPS				equ	26
E_NO_DEBUG			equ	27
E_NO_AUDIO_DRVR	equ	28
E_NO_AUDIO			equ	29
E_NO_CD_MAP			equ	30
E_INSERT_DISK		equ	31
E_NO_LANG_MAP		equ	32
E_INSERT_STARTUP	equ	33
E_NO_KBDDRV			equ	34
E_NO_HUNK			equ	35
E_NO_HANDLES		equ	36
E_NO_VIDEO			equ	37
E_CANT_FIND			equ	38
E_NO_PATCH			equ	39
E_NO_MUSIC			equ	40
E_CANT_LOAD			equ	41
E_NO_INSTALL		equ	42
E_RESRC_MISMATCH	equ	43
E_NOT_FOUND			equ	44
E_WRONG_RESRC		equ	45
E_LOAD_ERROR		equ	46
E_MAX_SERVE			equ	47
E_NO_DEBUGGER		equ	48
E_NO_MINHUNK		equ	49
E_NO_MEMORY			equ	50
E_NO_CONF_STORAGE	equ	51
E_BAD_PIC			equ	52
E_NO_HUNK_RES		equ	53
E_NO_RES_USE		equ	54
E_NO_HUNK_USE		equ	55
E_ARM_READ			equ	56
E_ARM_HANDLE		equ	57
E_NO_WHERE			equ	58
E_VIEW				equ	59
E_EXPLODE			equ	60
E_BUILD_LINE		equ	61
E_MIRROR_BUILD		equ	62
E_EAT_LINE			equ	63
E_LINE_LENGTH		equ	64
E_BAD_RECT			equ	65
E_AUDIOSIZE			equ	66
E_BAD_MINHUNK		equ	67
E_EXT_MEM			equ	68
E_POLY_AVOID		equ	69
E_POLY_MERGE		equ	70
E_MAX_PATCHES		equ	71
E_MAX_POINTS		equ	72
E_AVOID				equ	73
E_DRAWPIC			equ	74
E_FILL				equ	75
E_SAVEBITS			equ	76
E_NO_NIGHT_PAL		equ	77
E_MAGNIFY_FACTOR	equ	78
E_MAGNIFY_CEL		equ	79
E_MAGNIFY_SIZE		equ	80
E_BAD_LANG			equ	81
E_NO_HEAP			equ	82
E_HEAP_ALLOC		equ	83
E_HUNK_ALLOC		equ	84
E_NO_HUNK_SPACE	equ	85
E_RET_HUNK			equ	86
E_ADDMENU			equ	87
E_DRAWMENU			equ	88
E_SETMENU			equ	89
E_GETMENU			equ	90
E_MENUSELECT		equ	91
E_MESSAGE			equ	92
E_BRESEN				equ	93
E_BAD_PALETTE		equ	94
E_DISPOSED_SCRIPT	equ	95
E_SCALEFACTOR		equ	96
E_NO_HEAP_MEM		equ	97
E_TEXT_PARAM		equ	98
E_TEXT_COLOR		equ	99
E_TEXT_FONT			equ	100
E_TEXT_CODE			equ	101
E_ZERO_SIZE			equ	102
E_FREE_PAGES		equ	103
E_BAD_HANDLE_OUT	equ	104
E_BAD_HANDLE		equ	105
E_MAX_PAGES			equ	106
E_OPEN_WINDOW		equ	107
E_XMM_READ			equ	108
E_XMM_WRITE			equ	109
E_PANIC_INFO		equ	110
E_CONFIG_STORAGE	equ	111
E_CONFIG_ERROR		equ	112
E_BADMSGVERSION	equ     113
E_MSGSTACKOVERFLOW	equ     114
E_SAMPLE_IN_SND   equ     115
E_MSGSTACKSTACKOVERFLOW	equ 116
E_MSGSTACKSTACKUNDERFLOW	equ 117
E_BAD_POLYGON		equ   118
E_INVALID_PROPERTY		equ	119

externdef theViewNum: word
externdef theLoopNum: word
externdef theCelNum: word

ReadErrMsg		proto		num:word, errMsg:ptr byte
SetAlertProc	proto		o:word, s:word
Panic				proto		errnum:word, args:vararg
RAlert			proto		errnum:word, args:vararg
DoPanic			proto		msg:ptr byte
DoAlert			proto		msg:ptr byte
PanicMsgOutput proto		errNum:word

endif

