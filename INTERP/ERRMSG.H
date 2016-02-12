//	errmsg.h		10/30/91
//		access to text error messages

#if !defined(ERRMSG_H)
#define ERRMSG_H

#if !defined(TYPES_H)
#include "types.h"
#endif

#define	E_DISK_ERROR		1	/* Messages 1-24 pre-loaded into buffer */
#define	E_CANCEL				2
#define	E_QUIT				3
#define	E_DISK_ERROR_MSG	4	/* Disk error messages 4-16 */
#define	E_ARM_CHKSUM		17
#define	E_LASTPRELOAD		24

#define	E_OOPS_TXT			25
#define	E_OOPS				26
#define	E_NO_DEBUG			27
#define	E_NO_AUDIO_DRVR	28
#define	E_NO_AUDIO			29
#define	E_NO_CD_MAP			30
#define	E_INSERT_DISK		31
#define	E_NO_LANG_MAP		32
#define	E_INSERT_STARTUP	33
#define	E_NO_KBDDRV			34
#define	E_NO_HUNK			35
#define	E_NO_HANDLES		36
#define	E_NO_VIDEO			37
#define	E_CANT_FIND			38
#define	E_NO_PATCH			39
#define	E_NO_MUSIC			40
#define	E_CANT_LOAD			41
#define	E_NO_INSTALL		42
#define	E_RESRC_MISMATCH	43
#define	E_NOT_FOUND			44
#define	E_WRONG_RESRC		45
#define	E_LOAD_ERROR		46
#define	E_MAX_SERVE			47
#define	E_NO_DEBUGGER		48
#define	E_NO_MINHUNK		49
#define	E_NO_MEMORY			50
#define	E_NO_CONF_STORAGE	51
#define	E_BAD_PIC			52
#define	E_NO_HUNK_RES		53
#define	E_NO_RES_USE		54
#define	E_NO_HUNK_USE		55
#define	E_ARM_READ			56
#define	E_ARM_HANDLE		57
#define	E_NO_WHERE			58
#define	E_VIEW				59
#define	E_EXPLODE			60
#define	E_BUILD_LINE		61
#define	E_MIRROR_BUILD		62
#define	E_EAT_LINE			63
#define	E_LINE_LENGTH		64
#define	E_BAD_RECT			65
#define	E_AUDIOSIZE			66
#define	E_BAD_MINHUNK		67
#define	E_EXT_MEM			68
#define	E_POLY_AVOID		69
#define	E_POLY_MERGE		70
#define	E_MAX_PATCHES		71
#define	E_MAX_POINTS		72
#define	E_AVOID				73
#define	E_DRAWPIC			74
#define	E_FILL				75
#define	E_SAVEBITS			76
#define	E_NO_NIGHT_PAL		77
#define	E_MAGNIFY_FACTOR	78
#define	E_MAGNIFY_CEL		79
#define	E_MAGNIFY_SIZE		80
#define	E_BAD_LANG			81
#define	E_NO_HEAP			82
#define	E_HEAP_ALLOC		83
#define	E_HUNK_ALLOC		84
#define	E_NO_HUNK_SPACE	85
#define	E_RET_HUNK			86
#define	E_ADDMENU			87
#define	E_DRAWMENU			88
#define	E_SETMENU			89
#define	E_GETMENU			90
#define	E_MENUSELECT		91
#define	E_MESSAGE			92
#define	E_BRESEN				93
#define	E_BAD_PALETTE		94
#define	E_DISPOSED_SCRIPT	95
#define	E_SCALEFACTOR		96
#define	E_NO_HEAP_MEM		97
#define	E_TEXT_PARAM		98
#define	E_TEXT_COLOR		99
#define	E_TEXT_FONT			100
#define	E_TEXT_CODE			101
#define	E_ZERO_SIZE			102
#define	E_FREE_PAGES		103
#define	E_BAD_HANDLE_OUT	104
#define	E_BAD_HANDLE		105
#define	E_MAX_PAGES			106
#define	E_OPEN_WINDOW		107
#define	E_XMM_READ			108
#define	E_XMM_WRITE			109
#define	E_PANIC_INFO		110
#define	E_CONFIG_STORAGE	111
#define	E_CONFIG_ERROR		112
#define	E_BADMSGVERSION	113
#define	E_MSGSTACKOVERFLOW 114
#define	E_SAMPLE_IN_SND   115
#define	E_MSGSTACKSTACKOVERFLOW 116
#define	E_MSGSTACKSTACKUNDERFLOW 117
#define  E_BAD_POLYGON     118
#define	E_INVALID_PROPERTY	119

void		InitErrMsgs(void);
char *	ReadErrMsg(int, char *);
void		SetAlertProc(boolfptr);
void		Panic(int, ...);
bool		RAlert(int, ...);
void		DoPanic(strptr);
bool		DoAlert(strptr);

extern int theViewNum;
extern int theLoopNum;
extern int theCelNum;

#endif
