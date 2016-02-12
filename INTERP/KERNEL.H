//	kernel.h
//		defines standard parameter macros and declares kernel routines 

#if !defined(KERNEL_H)
#define KERNEL_H

#if !defined(LIST_H)
#include	"list.h"
#endif

#if !defined(OBJECT_H)
#include	"object.h"
#endif

typedef	word		*kArgs;
#define	arg(n)	*(args+(n))
#define	kArgc		arg(0)
#define	argList	kArgs args
#define	argCount	kArgc

/* pseudo selectors for SetMenu and Display 		*/
/* must be duplicated by (define)'s in SYSTEM.SH 	*/
#define p_at			100
#define p_mode			101
#define p_color		102
#define p_back			103
#define p_style		104
#define p_font			105
#define p_width		106
#define p_save			107
#define p_restore		108
#define p_said			109
#define p_text			110
#define p_key			111
#define p_state		112
#define p_value		113
#define p_dispose		114
#define p_time			115
#define p_title		116
#define p_draw			117
#define p_edit			118
#define p_button		119
#define p_icon			120
#define p_noshow		121

/* Function codes for memory segment operations */
#define SAVE_FROM		0
#define RESTORE_TO	1
#define STRING			0

/* Function codes for memory allocation operations */
#define MNeedPtr 		1
#define MNewPtr 		2
#define MDisposePtr	3
#define MCopy			4
#define MReadWord		5
#define MWriteWord 	6

/* Function code for PriCoord operation */
#define PTopOfBand 1

/* Function codes for KPalVary */
#define  PALVARYSTART     0
#define  PALVARYREVERSE   1
#define  PALVARYINFO      2
#define  PALVARYKILL      3
#define  PALVARYTARGET    4
#define  PALVARYNEWTIME	  5
#define  PALVARYPAUSE	  6
#define  DOSETCLUT        1
#define	NOSETCLUT        0

/* Function codes for KRemapColors */
#define REMAPOFF 0
#define REMAPPERCENT 1
#define REMAPRANGE 2
#define REMAPGRAY 3
#define REMAPPERCENTGRAY 4

// sortNode used in Sort
typedef struct {
	Obj*	sortObject;
	int	sortKey;
} SortNode;

typedef struct {
	Node	node;
	word	nVal;
} KNode;

void	KAssertPalette(word *);
void	KResCheck(word *);
void	KLoad(word *);
void	KUnLoad(word *);
void	KDrawPic(word *);
void	KPicNotValid(word *);
void	KShow(word *);
void	KAnimate(word *);
void	KNewWindow(word *);
void	KDisposeWindow(word *);
void	KDrawControl(word *);
void	KEditControl(word *);
void	KHiliteControl(word *);
void	KSetCursor(word *);
void	KShakeScreen(word *);
void	KShiftScreen(word *);
void	KGlobalToLocal(word *);
void	KLocalToGlobal(word *);
void	KGetAngle(word *);
void	KGetDistance(word *);
void	KSinMult(word *);
void	KCosMult(word *);
void	KSinDiv(word *);
void	KCosDiv(word *);
void	KATan(word *);
void	KGetEvent(word *);
void	KParse(word *);
void	KSaid(word *);
void	KCantBeHere(word *);
void	KGetTime(word *);
void	KTextSize(word *);
void	KOnControl(word *);
void	KAddToPic(word *);
void	KDrawCel(word *);
void	KNumCels(word *);
void	KNumLoops(word *);
void	KCelWide(word *);
void	KCelHigh(word *);
void	KGetPort(void);
void	KSetPort(word *);
void	KSetNowSeen(word *);
void 	KIsItSkip(word *);
void	KDisplay(word *);
void	KRandom(word *);
void	KNewList(void);
void	KDisposeList(word *);
void	KNewNode(word *);
void	KFirstNode(word *);
void	KLastNode(word *);
void	KEmptyList(word *);
void	KNextNode(word *);
void	KPrevNode(word *);
void	KIsFirstNode(word *);
void	KIsLastNode(word *);
void	KNodeValue(word *);
void	KAddAfter(word *);
void	KAddToFront(word *);
void	KAddToEnd(word *);
void	KFindKey(word *);
void	KDeleteKey(word *);
void	KAbs(word *);
void	KSqrt(word *);
void	KReadNumber(word *);
void	KMemoryInfo(word *);
void	KPalVary(word *);
void	KScriptID(word *);
void	KDUMMY(void);
void	KIsObject(word *);
void	KRespondsTo(word *);
void	KClone(word *);
void	KDisposeClone(word *);
void	KDisposeScript(word *);
void	KWait(word *);
void	KFormat(word *);
void	KStrCmp(word *);
void	KStrLen(word *);
void	KStrCpy(word *);
void	KStrEnd(word *);
void	KStrCat(word *);
void	KDummy(void);		/* KShowObjs(word *);*/
void	KHaveMouse(void);
void	KGetCWD(word *);
void	KCoordPri(word *);
void	KStrAt(word *);
void	KFlushResources(word *);
void	KMemorySegment(word *);
void	KMemory(word *);
void	KMergePoly(word *);
void	KAvoidPath(word *);
void	KListOps(word *);
void	KFileIO(word *);
void	KSort(word *);
void  bsort(SortNode*, int);
void	KLock(word *);
void  KShowMovie(word *);
void  KSetVideoMode(word *);
void	KSetQuitStr(word *);
void	KDbugStr(word *);
void	KDisposeScript(kArgs args);
void	KRemapColors(word *);

extern int noShowBits;

#endif

