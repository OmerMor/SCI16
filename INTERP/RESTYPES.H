/* resource types and subtypes */
#ifndef	RESTYPES_H

#define	RESTYPES_H

/* resource types */
#define	RES_BASE		0x80
#define	RES_VIEW		0x80
#define	RES_PIC		0x81
#define	RES_SCRIPT	0x82
#define	RES_TEXT		0x83
#define	RES_SOUND	0x84
#define	RES_MEM		0x85
#define	RES_VOCAB	0x86
#define	RES_FONT		0x87
#define	RES_CURSOR	0x88
#define	RES_PATCH	0x89
#define	RES_BITMAP	0x8a
#define	RES_PAL		0x8b
#define	RES_WAVE		0x8c
#define	RES_AUDIO	0x8d
#define	RES_SYNC		0x8e
#define	RES_MSG		0x8f
#define	RES_MAP		0x90
#define	RES_HEAP		0x91
#define	RES_AUDIO36	0x92
#define	RES_SYNC36	0x93
#define	RES_XLATE	0x94

#define	NRESTYPES (RES_XLATE - RES_BASE + 1)

#endif
