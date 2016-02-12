/* The debugger for the Script pseudo-machine.
 */

#include "altres.h"
#include	"audio.h"
#include "config.h"
#include	"ctype.h"
#include "debug.h"
#include "debugasm.h"
#include "dialog.h"
#include	"dos.h"
#include	"errmsg.h"
#include	"event.h"
#include	"fardata.h"
#include "graph.h"
#include	"info.h"
#include	"io.h"
#include "kerndisp.h"
#include	"memmgr.h"
#include	"opcodes.h"
#include "resname.h"
#include	"resource.h"
#include "savevars.h"
#include	"sci.h"
#include	"script.h"
#include	"selector.h"
#include	"sound.h"
#include "start.h"
#include "stdio.h"
#include	"stdlib.h"
#include	"string.h"
#include	"text.h"

#define	DEBUGFONT		999
#define	SIZEWINDOW		0
#define  ANYVALUE       -1

int		newRoomNum;
bool		trackResUse = FALSE;

#define	DBGTOP			2
#define	DBGWIDTH			140
#define	DBGHEIGHT		50
#define	DEBUG_BUF_SIZE	2000

#ifdef DEBUG
	global bool		traceOn;
	global bool		trace;
	global bool		findRet=FALSE;
	global uword	*lastSp;

	global Handle	proBuffer;
	global bool		proOn;
	global uint		proIndex;
	global uint		proBufSize;

	global bool		trackHunkUse = FALSE;

	global bool		isDebugVersion = 1;
	global int     debugWinX = 319-DBGWIDTH;
	global int     debugWinY = 10;

	static char		*debugBuf = NULL;

	static char* near	MakeDebugBuf(void);
	static void near	LoseDebugBuf(void);
#else
	global bool		isDebugVersion = 0;
#endif

static REventRecord	near dbgEvent;

global RWindow	* SizedWindow(str, title, wait)
strptr	str, title;
bool		wait;
{
	return (DebugWindow(str, title, SIZEWINDOW, -1, -1, wait));
}

static RWindow *
DWindow(RRect *fr, strptr title, word type, word pri, word vis)
{
	RWindow	*w;
	uint		oldBack, oldText;

	oldBack = winTitleBackColor;
	oldText = winTitleTextColor;
	winTitleBackColor = LGREY;
	winTitleTextColor = BLACK;

	w = RNewWindow(fr,NULL, title, type, pri, vis);
	winTitleBackColor = oldBack;
	winTitleTextColor = oldText;

	return w;
}

global RWindow	* DebugWindow(str, title, size, x, y, wait)
strptr	str, title;
int		size, x, y;
bool		wait;
{
	RWindow	*w;
	RRect		sr;
	int		titleWidth;
	word		oldFont;

	oldFont = GetFont();
	RSetFont(0);
	titleWidth = RStringWidth(title);
	RSetFont(oldFont);

	/* Bring screen intensity up to max so that message can be seen. */
	SetPalIntensity((RPalette far *) &sysPalette, 0, 255, 100);
	SetCLUT((RPalette far *) &sysPalette,FALSE);

	RTextSize(&sr, str, DEBUGFONT, size);
	if (sr.right < titleWidth)
		sr.right = titleWidth;
	CenterRect(&sr);
	MoveRect(&sr, (x == -1)? sr.left : x, (y == -1)? sr.top : y);
	RInsetRect(&sr, -4, -4);
	w = DWindow(&sr, title, title? TITLED : 0, -1, 1);
	RCopyRect(&rThePort->portRect, &sr);
	RInsetRect(&sr, 4, 4);
	RTextBox(str, 1, &sr, 0, DEBUGFONT);
	if (!wait)
		return (w);
	else
      {
		while (!RGetNextEvent(mouseDown | keyDown, &dbgEvent))
			;
		RDisposeWindow(w, TRUE);
		return (NULL);
		}
}



#ifndef DEBUG

#ifdef LINT_ARGS
	void NoDebugError(int);
#else
	void NoDebugError();
#endif

global strptr ArgNameRead(arg)
strptr	arg;
{
	if (*(arg+1) == '"') {
		/* skip switch and argument inside double quote */
		for (++arg; *++arg != '"'; ) ;
	}
	return arg;
}

global void Debug(ip, sp) /* NoDebug stub */
Hunkptr	ip;		/* instruction pointer */
uword		*sp;		/* stack pointer */
   {ip; sp; NoDebugError(1);}

global void KInspectObj(args) /* NoDebug stub */
word		*args;
   {args; NoDebugError(3);}

global void KProfiler(void) /* NoDebug stub */
   {NoDebugError(4);}

global void KSetDebug(args) /* NoDebug stub */
word	*args;
   {args; NoDebugError(5);}

global void KShowFree(void) /* NoDebug stub */
   {NoDebugError(6);}

global void KShowSends(args) /* NoDebug stub */
word	*args;
   {args; NoDebugError(7);}

global void ShowSends(wait) /* NoDebug stub */
bool		wait;
	{wait;}
	
global void Resources(void) /* NoDebug stub */
   {NoDebugError(8);}

global void SetDebug(on) /* NoDebug stub */
bool		on;
   {on; NoDebugError(9);}

global void ToggleDebug() /* NoDebug stub */
   {PError(NULL,NULL,NULL,NULL,NULL);}

global void WriteResUse(type,id) /* NoDebug stub */
int	type, id;
   {type; id; NoDebugError(10);}

global void CheckHunkUse(paragLocked)  /* NoDebug stub */
uint paragLocked;
   {paragLocked; return;}

global void PError(ip, sp, errCode, arg1, arg2) /* NoDebug stub (OOPS) */
char     *ip;
uword	   *sp;
int		errCode;
uint		arg1, arg2;
{
	char		title[25], msgTxt[400], msg[500];

	TermSndDrv();
	EndAudio();
	ip = 0;
	sp = 0;
	arg1 = 0;
	arg2 = 0;
	sprintf(msg, ReadErrMsg(E_OOPS_TXT, msgTxt), errCode, version);
	SizedWindow(msg, ReadErrMsg(E_OOPS, title), TRUE);
	exit(1);
}

/* This routine alerts QA'er that we should not have gotten here */
void NoDebugError(errorNumber)
int   errorNumber;
{
	Panic(E_NO_DEBUG, errorNumber);
}


#else /* #ifdef DEBUG (REACHES TO END OF FILE) */

typedef enum {
	PRO_OPEN,
	PRO_CLOSE,
	PRO_ON,
	PRO_OFF,
	PRO_CLEAR,
	PRO_REPORT,
	TRACE_ON,
	TRACE_OFF,
	TRACE_RPT
	}	ProCmd;

static int	MakeObjectsStr(strptr dest, bool showAddresses, Obj** firstObj,
					int nObjs);
static Obj*	FindObject(strptr name);

static	void		near	ShowVersion(void);
static   uint     near  CountHandles(void);
static	void		near	ShowObjects(bool showAddresses);
static	bool		near	InspectObj(Obj *);
static	strptr	near	GetSelectorName(int, strptr);
static	int		near	GetSelectorNum(strptr);
static	void     near	InspectHunk(char far *);
static	void		near	InspectList(Obj *);
static	void		near	EditProperty(Obj *, int);
static	void		near	EditValue(int *, strptr, strptr, ubyte);
static	int*		near	RequestPropAddr(Obj *, strptr, strptr);
static	void		near	DisposeDebugWin(void);
static	bool		near	IsObjBreakpoint(void);
static	bool		near	IsScrBreakpoint(Hunkptr);
static   bool     near  IsWatchHeapBreakpoint(void);
static   bool     near  IsWatchHunkBreakpoint(void);
static   bool     near  IsWatchKernelBreakpoint(Hunkptr);
static	void		near	ShowStackUsage(void);
static	strptr	near	GetObjName(Obj *);
static	int		near	DebugHelp(int n);

static	void		near	DebugInfo(Hunkptr, uword*);

static	void		near	ListOp(Hunkptr, strptr);
static	char		near	GetCommand(strptr);
static	bool		near	GetInput(strptr, strptr, int);
static	void		near	WriteHunkUse(uint);

#if defined(PROFILER)
static	void		near	Profiler(ProCmd command, strptr fileName, strptr comment);
#endif

#define	INSPECTWINDOW	220
#define	BIGWINDOW		290

#define	XMARGIN			4
#define	YMARGIN			4
#define	LINEINC			9
#define	Row(n)			(YMARGIN + LINEINC*(n))


#define	JUST_OP			0		/* Only operator -- no arguments */
#define	OP_ARGS			1		/* Operator takes arguments */
#define	OP_SIZE			2		/* Operator takes a size */

#define	ReadByte()	((uword) *(ip+1))
#define	ReadWord()	((uword) (*(ip+1) + *(ip+2) * 0x100))

/* Data
 */

static bool			showSends;

static uword 		bpOfs = 0;
static Handle		bpHndle = 0;
static uword      bpScriptNum = 0;
static uword      bpScriptOff = 0;
static Obj *		bpObj;
static int			bpSelect;
static uword      sbpScriptNum = 0;
static uword      sbpScriptOff = 0;
static int *      WatchHeapAddr = 0;
static char far * WatchHunkAddr = 0;
static int        WatchHunk = 0;
static int        WatchHeap = 0;
static int        WatchHeapValue;
static int        WatchHunkValue;
static int        WatchKernelOnce = 0;
static int        WatchKernelMany = 0;
static int        WatchKernelNumb;
static Obj *		sbpObj;
static int			sbpSelect;
static uword*		bpSp;
static bool			skipCurrent;

static RRect		dRect = {DBGTOP, 0, DBGTOP + Row(6) + YMARGIN, DBGWIDTH};
static RWindow *	debugWin;
static RWindow *	varWin;
static RWindow *	sendWin;
static RWindow *	errorWin;

static char			useName[80] = {0};
static char			hunkUseName[80] = {0};

global strptr ArgNameRead(arg)
strptr	arg;
{
	char c, *s;

	switch (*arg)
		{
		case 'U':
					trackHunkUse = TRUE;
					s = hunkUseName;
					strcpy(s,"hunk.use"); /*default name*/
					break;
		case 'u': 
					trackResUse = TRUE;
					s = useName; 
					strcpy(s,"resource.use"); /*default name*/ 
					break;
		}
		
	if (*(arg+1) == '"')
		{
		arg += 2; /* skip switch and double quote */
		for (c = *arg; c != '"'; c = *++arg) {
			*s++ = c;
			*s = '\0';
			}
		}
	return arg;
}

global void ToggleDebug()
/* Switch the current state of debugging.
 */
{
	if (!trace)
      {
		PauseSnd(NULL_OBJ, TRUE);
		trace = TRUE;
		DebugOn();
		}
	else
		{
		trace = FALSE;
		PauseSnd(NULL_OBJ, FALSE);
		}
}

static char* near
MakeDebugBuf()
{
	if (! debugBuf)
		debugBuf = NeedPtr(DEBUG_BUF_SIZE);
	return (debugBuf);
}

static void near
LoseDebugBuf()
{
	if (debugBuf) {
		DisposePtr(debugBuf);
		debugBuf = NULL;
	}
}

global void KSetDebug(args)
word	*args;
{
	SetDebug(TRUE);
	/* Return a value if one was passed. */
	if (argCount) acc = arg(1);
}

global void SetDebug(on)
bool		on;
{
	trace = on;
	if (trace)
      {
		PauseSnd(NULL_OBJ, TRUE);
		DebugOn();
		}
	else
      {
		DisposeDebugWin();
		/* only if there are no breakpoints can we turn debug off */
		if	
         (
			!findRet				   &&
      	bpHndle==0			   &&
			bpOfs==0				   &&
			bpObj==0				   &&
		   bpScriptOff==0		   &&
			sbpObj==0			   &&
		   sbpScriptOff==0      &&
		   WatchHunk==0         &&
		   WatchHeap==0         &&
         WatchKernelOnce==0   &&
         WatchKernelMany==0
			)

			DebugOff();

		LoseDebugBuf();
		PauseSnd(NULL_OBJ, FALSE);
		}
}

global void Debug(ip, sp)
Hunkptr	ip;		/* instruction pointer */
uword		*sp;		/* stack pointer */
{
	register	word	arg;
	register	ubyte	opCode, cmd;
	char		      argStr[80];
   uint           esHunk;
   char far       *essiHunk;
	Obj				**theBpObj;
	int				*theBpSelect;

	--ip;
	if (!trace && skipCurrent && !findRet && !IsScrBreakpoint(ip)) 
      {
		if (sp > bpSp || IsObjBreakpoint())
			return;
		skipCurrent = FALSE;
		}
	if 
      (
		trace                                             ||
		IsScrBreakpoint(ip)                               ||
		IsObjBreakpoint()                                 ||
      IsWatchHeapBreakpoint()                           ||
      IsWatchHunkBreakpoint()                           ||
      IsWatchKernelBreakpoint(ip)                         ||
		findRet                                           ||
		(
      (bpHndle == scriptHandle)    &&
      (bpOfs == (uint) ((long) ip & (long) 0xffff)) 
      )
		)

		{
		if (
         findRet && 
         !IsObjBreakpoint() &&
         !IsScrBreakpoint(ip) && 
         !IsWatchHeapBreakpoint() && 
         !IsWatchHunkBreakpoint() &&
         !IsWatchKernelBreakpoint(ip)
         )
		   {
			opCode=*ip;
			if((opCode & ~OP_BYTE) != OP_return)
            {
            return;
            }
			else
			   {
				findRet = FALSE;
			   }
		   }
		/* If we didn't turn debug on, it's on now! */
		trace=TRUE;

		if (IsObjBreakpoint()) PauseSnd(NULL_OBJ, TRUE);
      bpHndle = 0;
		bpOfs = 0;
		bpObj = NULL_OBJ;
		bpSelect = -1;
      bpScriptOff = 0;
		
		/* test if we just broke at a sticky obj/method breakpoint */
		if (IsObjBreakpoint())
         {
			skipCurrent = TRUE;
			bpSp = sp;
			}

		DebugInfo(ip, sp);
		if (showSends) ShowSends(FALSE);
		/* Do commands.
		 */
		forever
         {
			argStr[0] = '\0';
			cmd = GetCommand(argStr);
			arg = atoi(argStr);
			switch (cmd)
            {
				case 'a':
				case 'c':
					arg = (cmd == 'a')? acc : Pseudo(object);
					sprintf(argStr, "$%x", arg);
					InspectObj((Obj *) Native((arg&~1)));
					break;

				case 'C':
					sbpObj = NULL_OBJ;
					sbpSelect = -1;
               sbpScriptOff = 0;
               WatchKernelMany = 0;
               WatchHeap = FALSE;
               WatchHunk = FALSE;
					SetDebug(FALSE);
					return;

				case 'b':
				case 'B':
					theBpObj = (cmd == 'b') ? &bpObj : &sbpObj;
					theBpSelect = (cmd == 'b') ? &bpSelect : &sbpSelect;

			      while (!arg) {
					  arg = Pseudo(FindObject(argStr));
					  if (!arg && !GetInput(argStr, "Object not found", 30))
                    break;
               }
               if (!arg)
               	break;

					*theBpObj = (Obj *) (arg & ~1);

				GetSelect:
					argStr[0] = '\0';
					if (GetInput(argStr, "on method", 40))
                  {
						*theBpSelect = GetSelectorNum(argStr);
						if (*theBpSelect == -1 || !RespondsTo(*theBpObj, *theBpSelect))
                     {
							DebugWindow("not selector for object",
                        0, -1, -1, -1, TRUE);
							goto GetSelect;
							}
						}
					else
						*theBpSelect = -1;
					if (IsObjBreakpoint())
                  {
						skipCurrent = TRUE;
						bpSp = sp;
						}
					else
						skipCurrent = FALSE;
					if(cmd == 'b')
						{
						SetDebug(FALSE);
						return;
						}
					else
						break;

				case 'd':
					if (arg == 0) break;
					esHunk = atoi(argStr);
					argStr[0] = '$';
					argStr[1] = '\0';
					if (GetInput(argStr, "si", 6))
                  essiHunk = (char far *) ((long) esHunk << 16);
                  essiHunk += atoi(argStr);
					   InspectHunk(essiHunk);
					break;

				case 'D':
					SetDebug(FALSE);
					return;

				case 'e':
					if (arg == 0) break;
					EditValue((int *) atoi(argStr), "HEAP at", argStr, cmd);
					break;

				case 'f':
					KShowFree();
					break;

				case 'g':
					EditValue((int*) (globalVar + arg), "Global", argStr, cmd);
					break;

				case 'i':
			      while (!arg) {
					  arg = Pseudo(FindObject(argStr));
					  if (!arg && !GetInput(argStr, "Object not found", 30))
                    break;
               }
               if (!arg)
               	break;

					arg &= ~1;			/* Even boundaries only	*/
					InspectObj((Obj *) Native(arg));
					break;

				case 'k':
               WatchKernelOnce = TRUE;
					WatchKernelNumb = (int) atoi(argStr);
					SetDebug(FALSE);
					return;

				case 'K':
               WatchKernelMany = TRUE;
					WatchKernelNumb = (int) atoi(argStr);
					break;

				case 'l':
					EditValue(localVar + arg, "Local", argStr, cmd);
					break;

				case 'n':
					bpScriptNum = arg;
					argStr[0] = '$';
					argStr[1] = '\0';
					if (GetInput(argStr, "Script offset", 6))
					   bpScriptOff = atoi(argStr);
               if (!bpScriptOff) bpScriptOff = ANYVALUE;
					SetDebug(FALSE);
					return;
				
				case 'N':
					sbpScriptNum = arg;
					argStr[0] = '$';
					argStr[1] = '\0';
					if (GetInput(argStr, "Script offset", 6))
					   sbpScriptOff = atoi(argStr);
               if (!sbpScriptOff) sbpScriptOff = ANYVALUE;
					break;

				case 'o':
				case 'O':
					ShowObjects(cmd == 'O');
					break;

				case 'q':
					exit(0);

				case 'r':
					Resources();
					break;

				case 'R':
					findRet = TRUE;
					SetDebug(FALSE);
					return;
				
				case 's':
					showSends = !showSends;
					if (showSends)
						ShowSends(FALSE);
					else 
                  {
						lastSp = 0;
						RDisposeWindow(sendWin, TRUE);
						sendWin = NULL;
						}
					break;

				case 'S':
					ShowStackUsage();
					break;

				case 't':
               WatchHeap = TRUE;
					WatchHeapAddr = (int *) atoi(argStr);
               WatchHeapValue = *WatchHeapAddr;
					break;

				case 'T':
               WatchHunk = TRUE;
					if (arg == 0) break;
               WatchHunkAddr = (char far *) ((long) atoi(argStr) << 16);
					argStr[0] = '$';
					argStr[1] = '\0';
					if (GetInput(argStr, "T-Off", 6))
                  WatchHunkAddr += atoi(argStr);
               WatchHunkValue = *WatchHunkAddr;
					break;

				case '\t':
					/* Trace, but skip over procedure calls.
					 */
               bpHndle = scriptHandle;
					opCode = *ip;
					/* turn debug off temporarily and set breakpoint */
					trace = FALSE;
					switch (opCode & ~OP_BYTE) 
                  {
						case OP_self:
						case OP_send:
							bpOfs = (uint) ((long) ip & (long) 0xffff) + 2;
							break;

						case OP_call:
						case OP_callb:
						case OP_super:
							bpOfs = (uint) ((long) ip & (long) 0xffff) + ((opCode & OP_BYTE)? 3 : 4);
							break;

						case OP_calle:
							bpOfs = (uint) ((long) ip & (long) 0xffff)+ ((opCode & OP_BYTE)? 4 : 6);
							break;

						default:
							/* Go to next opcode */
							trace = TRUE;
							break;
						}
					return;

				case 'v':
					ShowVersion();
					break;

				case 'w':
	            RDisposeWindow(debugWin, TRUE);
	            debugWin = NULL;
               if (debugWinY == 10)
                  debugWinY = 189 - DBGHEIGHT;
               else
                  if ((debugWinY != 10) && (debugWinX == 319-DBGWIDTH))
                     debugWinX = 0;
                  else
                     {
                     debugWinX = 319-DBGWIDTH;
                     debugWinY = 10;
                     }
		         DebugInfo(ip, sp);
					break;

				case '\r':
					/* Go to next opcode */
					return;

				case '?':
					if(DebugHelp(0))
						DebugHelp(1);
					break;
				}
			}
		}
}

global void KShowSends(args)
word	*args;
{
	ShowSends(TRUE);
	if (argCount)
		acc = arg(1);
}

global void ShowSends(wait)
bool		wait;
{
	uword		*sp, *ss;
	char		*stackMsg, select[80];
	strptr	mp;
	Obj		*object;
	register	strptr	namePtr, msgPtr;

	/* null out initial string */
	stackMsg = MakeDebugBuf();
	stackMsg[0] = 0;
	/* account for clipped send stack */
	if ((sp = ssPtr) >= &ssEnd)
      {
		sp = &ssEnd - 2;
		strcpy(stackMsg, "*** Stack clipped. ***\n");
		}
	if (sendWin)
      {
		if (sp == lastSp)
			{
			LoseDebugBuf();
			return;
			}
		else
         {
			lastSp = sp;
			RDisposeWindow(sendWin, TRUE);
			sendWin = NULL;
			}
		}
	ss = sendStack;
	if (sp < ss)
      strcpy(stackMsg, "No send stack.");
	else {
		if (sp > (ss + 40))
			ss = sp - 38;			/* Max of 20 messages on screen at a time	*/
		for (mp = stackMsg + strlen(stackMsg) ; sp >= ss ; sp -= 2) {
         *mp++ = '(';
			object = (Obj *) Native(*sp);
			namePtr = GetObjName(object);
			if (!namePtr || !IsPrintStr(namePtr))
         	mp += sprintf(mp, "$%04x ", *sp);
         else
         	mp += sprintf(mp, "%s ", namePtr);
			msgPtr = GetSelectorName(*(sp+1), select);
			if (!msgPtr)
         	mp += sprintf(mp, "%d", select);
         else
         	mp += sprintf(mp, "%s", msgPtr);
         strcpy(mp, ":)\n");
         mp += 3;
		}
	}
	if (ss != sendStack)
		strcat(mp, "*** Stack clipped. ***\n");
	sendWin = DebugWindow(stackMsg, "Send Stack",
	   SIZEWINDOW, 4, 14, wait);
	LoseDebugBuf();
}


static void near
ShowVersion(void)
{
	char	str[200];

	sprintf(str, "Version %s\nMade on %s at %s\nin %s\nby %s\n%s",
				version, makeDate, makeTime, makeLocation, maker, makeComment);
	SizedWindow(str, "Version Info", TRUE);
}

void
PError(memptr ip, uword* sp, int errCode, uint arg1, uint arg2)
{
	char		str[300];
	char		select[40];
	strptr	object;

	TermSndDrv();
	EndAudio();

	switch (errCode) 
      {
		case E_BAD_DISPATCH:
			sprintf(str, "Dispatch number too large: %d", arg1);
			break;
		case E_BAD_OPCODE:
			sprintf(str, "Bad opcode: $%x", arg1);
			break;
		case E_BAD_KERNAL:
			sprintf(str, "Kernal entry # too large: %d", arg1);
			break;
		case E_LOAD_CLASS:
			sprintf(str, "Can't load class %d", arg1);
			break;
		case E_NOT_OBJECT:
			sprintf(str, "Not an object: $%x", arg1);
			break;
		case E_ZERO_DIVIDE:
			sprintf(str, "Attempt to divide by zero.");
			break;
		case E_BAD_SELECTOR:
			GetSelectorName(arg2, select);
			object = GetObjName((Obj *) Native(arg1));
			if (!object)
				object = "object";
			sprintf(str, "'%s' is not a selector for %s.", select, object);
			break;
		case E_STACK_BLOWN:
			strcpy(str, "Stack overflow.");
			break;
		case E_ZERO_MODULO:
			strcpy(str, "Zero modulo.");
			break;
		case E_LEFT_CLONE:
			sprintf(str, "Clone without script--> %d", arg1);
			break;
		case E_NO_DONGLE:
			sprintf(str, "Your global #101 is not set to %d", arg1);
			break;
		case E_PACKHANDLE_HEAP:
			sprintf(str, "PackHandle failure, duplicate table error at $%x in heap", arg1);
			break;
		case E_PACKHANDLE_HUNK:
			sprintf(str, "PackHandle failure, checksum error in loadlink at segment $%x", arg1);
			break;
		case E_PACKHANDLE_FAILURE:
			sprintf(str, "PackHandle failure, missing handle is for $%x segment", arg1);
			break;
		case E_ODD_HEAP_RETURNED:
			sprintf(str, "Heap failure, attempt to return heap at odd address. Address given is $%x ", arg1);
			break;
		case E_INVALID_PROPERTY:
			sprintf(str, "Invalid property %d", arg1);
			break;
		}
	trace = TRUE;
	errorWin = SizedWindow(str, "PMachine", TRUE);
	Debug(ip, sp);
	exit(1);
}

global void KShowFree()
{
	char	*str;

	str = MakeDebugBuf();

	str[0] = '\0';
	ShowFreeList(str);
	errorWin = DebugWindow(str, "Free Heap", BIGWINDOW, -1, -1, TRUE);
	LoseDebugBuf();
}

global void KInspectObj(args)
word		*args;
{
	InspectObj((Obj *) Native(arg(1)));
	if (argCount == 2)
		/* Return a value if one was passed.
		 */
		acc = arg(2);
}


global void WriteResUse(type,id)
int	type, id;
{
	int	fd;
	char	str[40];

	
	if ((fd = open(useName, 2)) != -1 ||
		 (fd = creat(useName, 0)) != -1
		)
		{
			lseek(fd, 0L, LSEEK_END);				/* seek to end of file */
			sprintf(str, "rm%03d  %s\t%03d\r\n", newRoomNum, ResName(type), id);
			write(fd, str, strlen(str));
			close(fd);
		}
	else
		{
			Panic(E_NO_RES_USE, useName);
		}
}

static uint near lastRoomNum, thisRoomMax, maxScriptNum;

global void CheckHunkUse(paragLocked) 
uint paragLocked;
   {
	LoadLink far * *scan;
	uword totalUnlocked,total;

	if (!trackHunkUse)
		return;

	/* search loadList for this type and id */
	for	(
			totalUnlocked = 0, 
			scan = (LoadLink far * *) Native(FFirstNode(&loadList));
			
			scan;
			
			scan = (LoadLink far * *) Native(FNextNode(Pseudo(scan)))
			)
	   {
		if (((*scan)->lock != LOCKED) && !((*scan)->altResMem))
			totalUnlocked += (((*scan)->size+15)/16) + 2;
		}
   total = hunkAvail - totalUnlocked - FreeHunk() + paragLocked;
		
	if ((total > minHunkSize) && (total > maxHunkUsed))	/* new high mark */
			WriteHunkUse(total);						/* write it always */

	if (newRoomNum != lastRoomNum)				/* detect room change */
		{
		if (thisRoomMax > minHunkSize)
			WriteHunkUse(thisRoomMax);				/* write max from last room */
		lastRoomNum = newRoomNum;
		thisRoomMax = maxScriptNum = 0;
		}
	
	if (total > thisRoomMax)
		{
		thisRoomMax = total;
		maxScriptNum = thisScript;
		}
		
	if (total > maxHunkUsed)				/* new high water mark */
		maxHunkUsed = total;					/* maintain it */
	}

static void near WriteHunkUse(total)
uint total;
	{
	int	fd;
	char	str[160];

	if ((fd = open(hunkUseName, 2)) != -1 ||
		 (fd = creat(hunkUseName, 0)) != -1
		)
		{
		lseek(fd, 0L, LSEEK_END);				/* seek to end of file */
		if (newRoomNum != lastRoomNum)
			{
			sprintf(str,
			"%03uk used in room %03d, executing script %03d\r\n",
				total/64,lastRoomNum,maxScriptNum);
			strcat(str,"------------------------------------\r\n");
			}
		else
			sprintf(str, 
			"%03uk used in room %03d, executing script %03d%s\r\n",
				total/64, lastRoomNum,
				thisScript,
				total > maxHunkUsed ? " NEW MAX!" : "");
		write(fd, str, strlen(str));
		close(fd);
		}
	else
		{
		Panic(E_NO_HUNK_USE, hunkUseName);
		}
	}

static char near GetCommand(str)
register	strptr	str;
{
	register	char			c;
	REventRecord	event;

	forever {
		/* Wait for an event.
		 */
		while (!RGetNextEvent(keyDown | mouseDown, &event))
			if (!trace)		/* hit Alt-D */
				return ('D');
		if (varWin != NULL) 
         {
			RDisposeWindow(varWin, TRUE);
			varWin = NULL;
			}
		if (event.type == mouseDown) return ('\r');
		c = (char) event.message;
		if(event.modifiers==ctrl)
			switch (c)
				{
				case 2: /* <ctrl>B */
					DebugWindow("Clearing sticky object/method breakpoint", 0, -1, -1, -1, TRUE);
					sbpObj = (Obj *) 0;
					sbpSelect = -1;
					break;
				case 11: /* <ctrl>K */
					DebugWindow("Clearing sticky kernel breakpoint", 0, -1, -1, -1, TRUE);
               WatchKernelMany = 0;
					break;
				case 14: /* <ctrl>N */
					DebugWindow("Clearing sticky script/offset breakpoint", 0, -1, -1, -1, TRUE);
               sbpScriptOff = 0;
					break;
				case 20: /* <ctrl>T */
					DebugWindow("Clearing trace words for HEAP and HUNK", 0, -1, -1, -1, TRUE);
               WatchHeap = FALSE;
               WatchHunk = FALSE;
					break;
				}
		else
			switch (c)
         	{
				case 'b':
				case 'B':
				case 'i':
					if (GetInput(str, (c == 'i')? "Inspect" : "Break in", 40))
						return (c);
					break;

				case 'd':
					str[0] = '$';
					str[1] = '\0';
					if (GetInput(str, " es ", 6))
						return (c);
					break;

				case 'e':
					str[0] = '$';
					str[1] = '\0';
					if (GetInput(str, "addr", 6))
                  return (c);
					break;

				case 'g':
				case 'l':
					if (GetInput(str, "Variable number", 5))
						return (c);
					break;

				case 'k':
				case 'K':
					if (GetInput(str, "K-Numb",6))
						return (c);
					break;

				case 'n':
				case 'N':
					if (GetInput(str, "Script number",6))
						return (c);
					break;

				case 't':
					str[0] = '$';
					str[1] = '\0';
					if (GetInput(str, "T-Addr",6))
						return (c);
					break;

				case 'T':
					str[0] = '$';
					str[1] = '\0';
					if (GetInput(str, "T-Seg",6))
						return (c);
					break;

				default:
					return (c);
				}
		}
}

static bool near IsObjBreakpoint()
{
	return
      (
			/* we may be beyond end of send stack */            	 	
			ssPtr < &ssEnd                                 &&
			/* If running without -d option, return no breakpoint set */ 
			*ssPtr                                         &&
			(
				(
					bpObj == (Obj *) *ssPtr         &&       	   	
					(bpSelect == *(ssPtr+1) || bpSelect == -1)
				)                                           ||
				(
					sbpObj == (Obj *) *ssPtr        &&       	   	
					(sbpSelect == *(ssPtr+1) || sbpSelect == -1)
				)
			)
		);
}

static bool near IsScrBreakpoint(ip)
Hunkptr	ip;		/* instruction pointer */
{

	return
		(
			(bpScriptNum == thisScript)
         && 
         (
            (bpScriptOff == (uint) ((long) ip & (long) 0xffff))
            ||
            (bpScriptOff == (uint) ANYVALUE)
         )
      )                                                    
      ||
		(
			(sbpScriptNum == thisScript)
         &&
         (
            (sbpScriptOff == (uint) ((long) ip & (long) 0xffff))
            ||
            (sbpScriptOff == (uint) ANYVALUE)
         )
      );
}

static bool near IsWatchHeapBreakpoint()
{
   if (WatchHeap)
      {
      if (WatchHeapValue != *WatchHeapAddr)
         {
         WatchHeap = FALSE;
	      return TRUE;
         }
      }
	return FALSE;
}

static bool near IsWatchHunkBreakpoint()
{
   if (WatchHunk)
      {
      if (WatchHunkValue != *WatchHunkAddr)
         {
         WatchHunk = FALSE;
	      return TRUE;
         }
      }
	return FALSE;
}

static bool near IsWatchKernelBreakpoint(ip)
Hunkptr	ip;
{
	char	opCode;
   if (WatchKernelOnce || WatchKernelMany)
      {
		opCode = *ip;
      if ((opCode & ~OP_BYTE) == OP_callk)
         {
			switch (opCode) 
            {
				case OP_callk:
               if (*(int far *)(ip+1) == WatchKernelNumb)
                  {
                  WatchKernelOnce = FALSE;
	               return TRUE;
                  }
				   break;
				case OP_callk+1:
               if ((int) *(ip+1) == WatchKernelNumb)
                  {
                  WatchKernelOnce = FALSE;
	               return TRUE;
                  }
				   break;
            }
         }
      }
	return FALSE;
}

static bool near GetInput(str, title, length)
register	strptr	str;
strptr	title;
int		length;
{
	RWindow 		*wind;
	RRect 			r, box;
	int			cursor, width, oldFont, maxLen;
	bool			retVal, done;
	REventRecord	evt;

	oldFont = GetFont();
	width = RTextWidth(title, 0, strlen(title), oldFont);
	RSetFont(DEBUGFONT);
	maxLen = length * RCharWidth('m');
	if (width < maxLen) width = maxLen;
	RSetRect(&r, 0, 0, width, 10);
	CenterRect(&r);
	MoveRect(&r, r.left, 160);
	wind = DWindow(&r, title, TITLED | STDWIND, -1, 1);
	RSetFont(DEBUGFONT);
	RMoveTo(0, 1);
	ShowString(str);
	done = FALSE;
	cursor = strlen(str);
	RCopyRect(&rThePort->portRect, &box);
	RInsetRect(&box, 1, 1);
	DrawCursor(&box, str, cursor);
	forever
      {
		RGetNextEvent(allEvents, &evt);
		RGlobalToLocal(&evt.where);
		if (evt.type == keyDown)
         {
			switch (evt.message)
            {
				case ESC:
					*str = '\0';
					retVal = FALSE;
					done = TRUE;
					break;
				case CR:
					retVal = TRUE;
					done = TRUE;
					break;
				}
			}
			if (done) break;
			cursor = EditText(&box, str, cursor, length, &evt);
		}
	EraseCursor();
	RDisposeWindow(wind, TRUE);
	while (*str == ' ')  strcpy(str, str+1);
	RSetFont(oldFont);
	return (retVal);
}

static void near
DebugInfo(
	Hunkptr	ip,		// instruction pointer
	uword*	sp)		// stack pointer
{
	char		theStr[50];
	strptr	namePtr;

	/* Open a window if not open.
	 */
	if (debugWin == NULL)
      {
		CenterRect(&dRect);
		MoveRect(&dRect, debugWinX, debugWinY);
		debugWin = DWindow(&dRect, "Debug", TITLED, -1, 1);
		RPenMode(SRCCOPY);
		RSetFont(DEBUGFONT);
		}
	RSelectWindow(debugWin);
	if
      (
      object == NULL                           ||
      GetPropAddr(object, s_name) == NULL      ||
      (namePtr = GetObjName(object)) == NULL
      )
			namePtr = "";
	RMoveTo(XMARGIN, Row(0));
	sprintf(theStr, "%-30s", namePtr);
	PenColor(BLACK);
	DrawString(theStr);
	PenColor(BLACK);
	RMoveTo(XMARGIN, Row(1));
   /* in case debug window causes code movement */
	if (*((long *) scriptHandle) != 0)
      {
	      ip =  (Hunkptr)
               (
               ((char far *) *((long *) scriptHandle)) +
               (uint) ((long) ip & (long) 0xffff)
               );
      }
	ListOp(ip, theStr);
	DrawString(theStr);
	/* Write the info.
	 */
	RMoveTo(XMARGIN, Row(2));
	PenColor(BLACK);
	sprintf(theStr, "acc:%04x sp:%04x pp:%04x", acc, Pseudo(sp), Pseudo(parmVar));
	DrawString(theStr);
	/* Write the ip.
	 */
	RMoveTo(XMARGIN, Row(3));
	sprintf(theStr, "es:%04x%s si:%04x sn:%04d",
	   (int ) (((long) ip) >> 16), 
      defaultES ? "*" : " ",
      (int) ((long) ip & (long) 0xffff),
      thisScript);
	DrawString(theStr);
   /* Write the Global,Local,Temp addresses
    */
	RMoveTo(XMARGIN, Row(4));
	sprintf(theStr, "G:%04x L:%04x T:%04x", globalVar, localVar, tempVar);
	DrawString(theStr);
	/* Write the Version.
	 */
	PenColor(BLACK);
	RMoveTo(XMARGIN, Row(5));
	sprintf(theStr, "SCI Version %s", version);
	DrawString(theStr);
	/* Show what we have wrought.
	 */
	ShowBits(&rThePort->portRect, VMAP);
}

static void near ListOp(ip, theStr)
Hunkptr	ip;
register	strptr	theStr;
{
	register	ubyte		theOp;
	strptr	op, branchStr;
	char		opData[20];
	uword		theArg;
	bool		hasArgs, byteSize;
	int		i, length;

   theOp = *ip;
	byteSize = theOp & OP_BYTE;
	theOp &= ~OP_BYTE;

	length = GetFarData(OPCODE_VOCAB, theOp/2, (uchar*) opData);
	opData[length] = 0;
	hasArgs = GetWordP(opData) & OP_ARGS;
	op = &opData[2];

	if (byteSize)
		theArg = ReadByte();
	else
		theArg = ReadWord();

	if (!hasArgs)
		sprintf(theStr, "%s", op);
	else
		switch (theOp) {
			case OP_callk:
				sprintf(theStr, "%s  %s", op, kernelNames[theArg]);
				break;

			case OP_bt:
			case OP_bnt:
				if ((acc && (theOp == OP_bt)) || (!acc && (theOp == OP_bnt)))
					branchStr = "";
				else
					branchStr = "not ";
				sprintf(theStr, "%s  $%-4x will %sbranch", op, theArg, branchStr);
				break;

			default:
				sprintf(theStr, "%s  $%x", op, theArg);
				break;
			}

	// Pad the string to 30 chars.
	for (i = strlen(theStr) ; i < 30 ; ++i)
		theStr[i] = ' ';
	theStr[30] = '\0';
}

static strptr near
GetSelectorName(
	register	int		id,
	register	strptr	str)
{
	if (GetFarStr(SELECTOR_VOCAB, id, str) == NULL)
		sprintf(str, "%x", id);
	return str;
}

static int near GetSelectorNum(str)
strptr	str;
{
	register	int		i;
	char		selector[60];

	/* Keep trying new selector numbers until we get a NULL return
	 * for the string address -- this indicates that we've exceeded
	 * the maximum selector number.
	 */
	for (i = 0 ; GetFarStr(SELECTOR_VOCAB, i, selector) ; ++i)
		if (strcmp(str, selector) == 0)
			return (i);
	return (-1);
}

static void near
ShowObjects(
	bool	showAddresses)
{
	char	winTitle[25];
	int	numObjs;
	int	printedObjs = 0;
	char*	str = MakeDebugBuf();
   Obj*	curObj = 0;

   while (numObjs = MakeObjectsStr(str, showAddresses, &curObj, 50)) {
      printedObjs += numObjs;
	   sprintf(winTitle, "Objects: %d", printedObjs);
	   errorWin = DebugWindow(str, winTitle, BIGWINDOW, -1, -1, TRUE);
      if (dbgEvent.message == ESC)
			break;
   }
	LoseDebugBuf();
}

/* Function codes for StackUsage.
 */
#define	PROC_SIZE		0		/* size of processor stack */
#define	PROC_MAX			1		/* maximum processor stack used */
#define	PROC_CUR			2		/* current usage of processor stack */
#define	PM_SIZE			3		/* size of PMachine stack */
#define	PM_MAX			4		/* maximum PMachine stack used */
#define	PM_CUR			5		/* current usage of PMachine stack */

static void near ShowStackUsage()
{
	char	str[300];

	sprintf
		(str,
			"Proc\n  size:%d\n  max:%d\n  cur:%d\nPMach\n  size:%d\n \
         max:%d\n  cur:%d",
			StackUsage(PROC_SIZE), StackUsage(PROC_MAX), StackUsage(PROC_CUR),
			StackUsage(PM_SIZE), StackUsage(PM_MAX), StackUsage(PM_CUR)
		);
	SizedWindow(str, "Stack", TRUE);
}

#define INSP_BUFF_SIZE 2000

static bool near InspectObj(obj)
register	Obj	*obj;
{
	register		int	i;
	Dictptr		dict;
	int			numProps, val;
	REventRecord	event;
	RWindow		*win;
	int			top;
	bool			done;
	bool			ret;
	char			c, sepChar;
	Obj			*tmpObj;
	char			title[60], selector[40];	/*, data[INSP_BUFF_SIZE];*/
	char			*data;
	static	int	inspectLevel;
#define	TOPOFSHEX		30
#define	TOPOFSOBJ		0
#define	BIGGESTWINDOW	310
#define	MAXINSPECT	3

	data = MakeDebugBuf();
ReInspect:
	++inspectLevel;
	if (obj == NULL || !IsObject(obj)) {
	   top = TOPOFSHEX + inspectLevel*12;
		if (obj == NULL)
			obj = (Obj *) Native(1000);	/* NULL ptr will crash 68000	*/
		data[500] = data[0] = '\0';
		for (i = 0 ; i < 64 ; ) {
			c = *((strptr) obj + i);
			++i;
			sepChar = (char) ((i%16)? ' ' : '\n');
			sprintf(title, "%02x%c", c & 0xff, sepChar);
			strcat(data, title);
			sprintf(title, "%c %c", (' ' <= c && c < 127 && c != CTRL_CHAR)? c : '.', sepChar);
			strcat(&data[500], title);
			}
		strcat(data, "\n");
		strcat(data, &data[500]);
		sprintf(title, "Inspect :: $%04x", Pseudo(obj));
	   win = DebugWindow(data, title, INSPECTWINDOW, -1, top, FALSE);
		}
	else {
	   top = TOPOFSOBJ + inspectLevel*13;
		numProps = obj->size;
		ResLoad(RES_VOCAB, SELECTOR_VOCAB);		/* Make sure hunk doesn't move */
		dict =
         (Dictptr) *((Handle) ((Script*) Native(obj->classScript))->hunk);
      dict += ((int) obj->propDict)/2;
		sprintf(data, "super: %s\n",
			Native(obj->super) != NULL ? GetObjName((Obj *) Native(obj->super)) : "RootObj");

		for (i = 0 ; i < numProps ; ++i){
			GetSelectorName(*(dict+i), selector);
			val = *(((uword *)obj) + i);
			sprintf(title,
				((-100 <= val) && (val <= 1000))? "%s:%d  " : "%s:$%x  ",
				selector, *(((uword *) obj) + i));
			/* check for TOO much data Do early exit if over */
			if ( (strlen(data) + strlen(title)) >= INSP_BUFF_SIZE - 2)
				break;			
		
			strcat(data, title);
		}
		sprintf(title, "Inspect :: %s ($%x)", GetObjName(obj), Pseudo(obj));
      if (numProps > 50)
	      win = DebugWindow(data, title, BIGGESTWINDOW, -1, top, FALSE);
      else
	      win = DebugWindow(data, title, BIGWINDOW, -1, top, FALSE);
		}
	for (done = FALSE, ret = FALSE ; !done ; )
      {
		while (!RGetNextEvent(keyDown | mouseDown, &event))
			;
		if (event.type == mouseDown)
			done = TRUE;
		else
			switch (event.message)
            {
				case 'i':
				case 'e':
					if (inspectLevel < MAXINSPECT && IsObject(obj))
						EditProperty(obj, event.message);
					break;
				case 'c':
					if (inspectLevel < MAXINSPECT)
						InspectList(obj);
					break;
				case 't':
					if (inspectLevel < MAXINSPECT && IsObject(obj))
						EditProperty(obj, event.message);
					break;
				case 'T':
					if (inspectLevel < MAXINSPECT && IsObject(obj))
						EditProperty(obj, event.message);
					break;
				case '[':
				case ']':
					i = (event.message == ']')? 1 : 0;
               tmpObj = (Obj *) Native(*((int *) obj + i));
					if (tmpObj != NULL) {
						obj = tmpObj;
						RDisposeWindow(win, TRUE);
						--inspectLevel;
						goto ReInspect;
						}
					break;
				case UPARROW:
               obj = (Obj *) ((int *) obj - 8);
					RDisposeWindow(win, TRUE);
					--inspectLevel;
					goto ReInspect;
				case DOWNARROW:
               obj = (Obj *) ((int *) obj + 8);
					RDisposeWindow(win, TRUE);
					--inspectLevel;
					goto ReInspect;
				case LEFTARROW:
               obj = (Obj *) ((int *) obj - 1);
					RDisposeWindow(win, TRUE);
					--inspectLevel;
					goto ReInspect;
				case RIGHTARROW:
               obj = (Obj *) ((int *) obj + 1);
					RDisposeWindow(win, TRUE);
					--inspectLevel;
					goto ReInspect;
				case PAGEDOWN:
               obj = (Obj *) ((int *) obj + 32);
					RDisposeWindow(win, TRUE);
					--inspectLevel;
					goto ReInspect;
				case PAGEUP:
               obj = (Obj *) ((int *) obj - 32);
					RDisposeWindow(win, TRUE);
					--inspectLevel;
					goto ReInspect;
				case '?':
					if (inspectLevel < MAXINSPECT)
						DebugHelp(2);
					break;
				case CR:
					done = TRUE;
					break;
				case ESC:
					done = TRUE;
               ret  = TRUE;
					break;
				}
		}
	RDisposeWindow(win, TRUE);
	--inspectLevel;
	LoseDebugBuf();
	return (ret);
}

static void near InspectHunk(essiHunk)
char far *essiHunk;
{
	register		int	i;
	REventRecord	event;
	RWindow		*win;
	int			top;
	char			c, sepChar;
	char			title[60];
	char			*data;
	static	int	inspectLevel;
#define	TOPOFS		30
#define	MAXINSPECT	3

	data = MakeDebugBuf();
ReInspect:
	data[500] = data[0] = '\0';
	for (i = 0 ; i < 64 ; ) 
      {
		c = *(essiHunk + i);
		++i;
		sepChar = (char) ((i%16)? ' ' : '\n');
		sprintf(title, "%02x%c", c & 0xff, sepChar);
		strcat(data, title);
		sprintf(title, "%c %c", (' ' <= c && c < 127 && c != CTRL_CHAR)? c : '.', sepChar);
		strcat(&data[500], title);
		}
	strcat(data, "\n");
	strcat(data, &data[500]);
	sprintf(title, "Hunk at %04x:%04x",
	   (int ) (((long) essiHunk) >> 16), 
      (int) ((long) essiHunk & (long) 0xffff));
	top = TOPOFS + inspectLevel*12;
	++inspectLevel;
	win = DebugWindow(data, title, INSPECTWINDOW, -1, top, FALSE);
	while (!RGetNextEvent(keyDown | mouseDown, &event))
      ;
	if (event.type == mouseDown)
		return;
	switch (event.message)
      {
		case UPARROW:
			essiHunk -= 16;
			RDisposeWindow(win, TRUE);
			--inspectLevel;
			goto ReInspect;
		case DOWNARROW:
			essiHunk += 16;
			RDisposeWindow(win, TRUE);
			--inspectLevel;
			goto ReInspect;
		case LEFTARROW:
			--essiHunk;
			RDisposeWindow(win, TRUE);
			--inspectLevel;
			goto ReInspect;
		case RIGHTARROW:
			++essiHunk;
			RDisposeWindow(win, TRUE);
			--inspectLevel;
			goto ReInspect;
		case PAGEDOWN:
			essiHunk += 64;
			RDisposeWindow(win, TRUE);
			--inspectLevel;
			goto ReInspect;
		case PAGEUP:
			essiHunk -= 64;
			RDisposeWindow(win, TRUE);
			--inspectLevel;
			goto ReInspect;
		case CR:
         break;
		case ESC:
			break;
		}
	RDisposeWindow(win, TRUE);
	--inspectLevel;
	LoseDebugBuf();
	return;
}

static void near InspectList(obj)
register	Obj	*obj;
{
	List		*theList;
	register	ObjID		theNode;
	word		*elements;
	strptr	objName;
	char		msg[60], aName[60];

	if (!IsObject(obj))
      {
		objName = aName;
		sprintf(objName, "$%x", Pseudo(obj));
		}
	else
		objName = GetObjName(obj);
	elements = GetPropAddr(obj, s_elements);
	if (elements == NULL)
		sprintf(msg, "%s not a Collection.", objName);
	else
      {
		theList = (List *) Native(*elements);
		if (theList == NULL)
			sprintf(msg, "%s is empty.", objName);
		}

	if (elements == NULL || theList == NULL)
		SizedWindow(msg, objName, TRUE);
	else
      {
		for 
         (
         theNode = FirstNode(theList) ;
			theNode && IsObject((Obj *) Native(GetKey(theNode))) ;
			theNode = NextNode(theNode)
		   )
			if (InspectObj((Obj *) Native(GetKey(theNode)))) break;
		SizedWindow("end", objName, TRUE);
		}
}

static void near EditProperty(obj, edit)
register	Obj	*obj;
int		edit;
{
	char		selStr[40];
	register	strptr	objName;

	objName = GetObjName(obj);
	EditValue(RequestPropAddr(obj, objName, selStr), objName, selStr, (ubyte) edit);
}

static void near EditValue(addr, s1, s2, edit)
register	int		*addr;
strptr	s1, s2;
ubyte		edit;
{
	char		title[60], data[40];
	int		value;
	register	Obj	*objAddr;

	if (addr == NULL) return;
	value = *addr;
	sprintf(title, "%s :: %s", s1, s2);
	sprintf(data, ((-100 <= value) && (value <= 1000))? "%d" : "$%x", value);
	switch (edit)
      {
		case 'i':
		   objAddr = (Obj *) Native(value);
		   if (IsObject(objAddr))
			   InspectObj(objAddr);
		   else
            {
			   if (IsPrintStr((strptr) objAddr))
               {
				   strcat(data, "\n");
				   strcat(data, (strptr) objAddr);
				   }
			   SizedWindow(data, title, TRUE);
			   }
			break;
		case 'e':
		case 'g':
		case 'l':
		   objAddr = (Obj *) Native(value);
		   if (IsObject(objAddr))
			   strcpy(data, GetObjName(objAddr));
		   if (GetInput(data, title, 40))
            {
			   if (
               (isdigit(data[0]) || (data[0] == '$'))  ||
			      ((data[0] == '-') && isdigit(data[1]))
               )

               {
				   *addr = atoi(data);
               }
			   else
               {
				   objAddr = FindObject(data);
				   if (objAddr)
                  {
				      value = Pseudo(objAddr);
				      *addr = value;
				      }
				   else
				      value = NULL;
				   }
			   }
			break;
		case 't':
         WatchHeap = TRUE;
			WatchHeapAddr = addr;
         WatchHeapValue = value;
			DebugWindow("Setting Watch word breakpoint on property", 0, -1, -1, -1, TRUE);
			break;
		}
}

static int	* near RequestPropAddr(obj, objName, selStr)
Obj		*obj;
strptr	objName, selStr;
{
	char		title[60];
	register	int		selNum;
	int		*propAddr;

	selStr[0] = '\0';
	if (!GetInput(selStr, "Selector", 40)) return (NULL);
	selNum = GetSelectorNum(selStr);
	propAddr = GetPropAddr(obj, selNum);
	if (selNum == -1 || propAddr == 0)
      {
		sprintf(title, "%s: not selector for %s.", selStr, objName);
		SizedWindow(title, "Property", TRUE);
		return (NULL);
		}
	return (propAddr);
}

static int near DebugHelp(n)
int		n;
{
	char		*helpStr;
	RWindow	*w;
	REventRecord	event;
	char		name[13];
	char		path[100];
	int		fd;
	uint		fileLen;

	helpStr = MakeDebugBuf();
	sprintf(name, "DBGHELP.%03d", n);
	if ((fd = open(name, 0)) == -1) {
		GetExecDir(path);
		strcat(path, name);
		fd = open(path, 0);
	}
	if (fd != -1) {
		fileLen = (uint)lseek(fd, 0L, LSEEK_END);
		lseek(fd, 0L, LSEEK_BEG);
		read(fd, helpStr, fileLen);
		close(fd);
		helpStr[fileLen] = 0;
	} else {
		sprintf(helpStr, "Could not find help file %s", name);
	}
	PenColor(BLACK);
	w=SizedWindow(helpStr, "Help", FALSE);
	PenColor(BLACK);
	while (!RGetNextEvent(mouseDown | keyDown, &event))
		;
	RDisposeWindow(w, TRUE);
	LoseDebugBuf();
	if(event.message==ESC || fd==-1)
		return FALSE;
	else
		return TRUE;
}


static void near DisposeDebugWin()
{
	if (debugWin != NULL) RDisposeWindow(debugWin, TRUE);
	debugWin = NULL;
	if (sendWin != NULL) RDisposeWindow(sendWin, TRUE);
	sendWin = NULL;
}

static strptr near
GetObjName(
	Obj*	obj)
{
	strptr* name = GetPropAddr(obj, s_name);
   return name ? *name : NULL;
}

#define	PRO_SIZE		0x3f00

void
KProfiler(
	word*	args)
{
#ifdef PROFILER
	ProCmd	command;
	strptr	fileName, comment;

	if ((command = (ProCmd) arg(1)) == PRO_REPORT || command == TRACE_RPT)
      {
		fileName = (strptr) arg(2);
		comment = (argCount == 3)? (strptr) arg(3) : "";
		}
	else
		fileName = comment = NULL;
	Profiler(command, fileName, comment);
#else
	args = args;
#endif
}

#ifdef PROFILER
static void near Profiler(command, fileName, comment)
ProCmd	command;
strptr	fileName, comment;
{
	Obj		*obj;
	uint		selector;
	uint far	*pb;
	uint		pi, pl, count, totalCount;
	char		str[100], selStr[30];
	int		fd;

	switch (command) 
      {
		case PRO_OPEN:
			/* Get a buffer for storing the profiling information,
			 * the turn profiling on.
			 */
			proBufSize = 2 * sizeof(uint) * PRO_SIZE;
			if (!proBuffer)
				proBuffer = GetResHandle(proBufSize+4);
			ClearHandle(proBuffer);
			proIndex = 0;	
			break;

		case PRO_CLOSE:
 			/* Turn the profiler off, then deallocate the buffer.
			 */
			proOn = FALSE;
			DisposeServer(ProfileHandler);
			DisposeHandle(proBuffer);
			proBuffer = 0;
			break;

		case PRO_ON:
			proOn = TRUE;
			InstallServer(ProfileHandler, 1);
			break;

		case PRO_OFF:
			proOn = FALSE;
			DisposeServer(ProfileHandler);
			break;
			
		case PRO_CLEAR:
			/* Clear the profiler buffer.
			 */
			if (proBuffer)
            {
				proOn = FALSE;
				ClearHandle(proBuffer);
				proIndex = 0;
				proOn = TRUE;
				}
			break;
			
		case PRO_REPORT:
			if 
            ((fd = open(fileName, O_WRONLY)) == -1 &&
				 (fd = creat(fileName, 0)) == -1
				)
				{
				proOn = TRUE;
				return;
				}
			lseek(fd, 0L, LSEEK_END);		/* Seek to end of file. */
			/* Write the comment to the file.
			 */
			sprintf(str ,"\r\n***** %s\r\n", comment);
			write(fd, str, strlen(str));
			/* Write statistics out.
			 */
			pb = (uint far *) *proBuffer;		/* pointer to profiler buffer */
			pl = 0;						/* last profiler index */
			totalCount = 0;
			forever
            {
				/* Search for the next message in the buffer.
				 */
				for (pi = pl ;
					pi < 2*PRO_SIZE && *(pb+pi) == 0;
					pi += 2)
						;

				if (pi >= 2*PRO_SIZE) break;
				pl = pi;
				obj = (Obj *) Native(*(pb+pi));
				selector = *(pb+pi+1);
				count = 0;
				/* Now count occurences of this object/selector pair.
				 */
				for ( ; pi < 2*PRO_SIZE ; pi += 2)
               {
					if ((Obj *) Native(*(pb+pi)) == obj && *(pb+pi+1) == selector)
                  {
						*(pb+pi) = *(pb+pi+1) = 0;
						++count;
						}
					}
				/* Write out the line for this object/selector pair.
				 */
				totalCount += count;
				sprintf(str, "%6d   (%s %s:)\r\n",
					count,
					GetObjName(obj),
					GetSelectorName(selector, selStr));
				write(fd, str, strlen(str));
				}
			sprintf(str ,"\r\n***** Total count = %d\r\n", totalCount);
			write(fd, str, strlen(str));
			close(fd);
			break;

		case TRACE_ON:
			ClearHandle(proBuffer);
			proIndex = 0;
			traceOn = TRUE;
			break;

		case TRACE_OFF:
			traceOn = FALSE;
			break;

		case TRACE_RPT:
			if 
            ((fd = open(fileName, O_WRONLY)) == -1 &&
				 (fd = creat(fileName, 0)) == -1
				)
					return;
			lseek(fd, 0L, LSEEK_END);		/* Seek to end of file. */
			write(fd, comment, strlen(comment));
			write(fd, "\r\n", 1);
			pb = (uint far *) *proBuffer;
			pl = proIndex/2;
			for (pi = 0 ; pi < pl ; pi += 3)
            {
				count = *(pb+pi)/6;
				obj = (Obj *) Native(*(pb+pi+1));
				selector = *(pb+pi+2);
				while (count--)
					write(fd, "\t", 1);
				sprintf(str, "(%s %s:)\r\n",
					GetObjName(obj),
					GetSelectorName(selector, selStr));
				write(fd, str, strlen(str));
				}
			close(fd);
			break;
		}
}
#endif

static	char	resSym[] = "vpstnmwfcpbPCaSMMh";

global void Resources(void)
{
	#define NRES_SUMS_PER_LINE			3
	#define NMEMORY_SUMMARY_LINES		2
	#define NARM_KEY_LINES				1
	#define MAX_BUF						1500
	#define MIN_ENTRIES_LINE			8

	int		maxLines;
	int		maxResourceLines;
	char		*buf;
	char*		bp;
	RRect		rect;
	int		entriesOnLine;
	int		type;
	int		printCount;
	bool		first;
	char*		title;
	LoadLink far **loadLink;
	ulong		sizes[NRESTYPES];
	int		nResourceTypes;
	int		nResourceSummaryLines;
	bool		arm[NARMTYPES];
	int		nARMTypes;
	ulong		lockedSize=0;
   int		pointSize = GetPointSize();

	buf = MakeDebugBuf();
	/*	determine amount of memory used for each resource type
		and how many ARM types are in use
	*/
	for (type = 0; type < NRESTYPES; type++)
		sizes[type] = 0;
	for (type = 0; type < NARMTYPES; type++)
		arm[type] = FALSE;

	for (loadLink = (LoadLink far**) Native(FFirstNode(&loadList));
		  loadLink;
		  loadLink = (LoadLink far**) Native(FNextNode(Pseudo(loadLink)))) {
		sizes[(*loadLink)->type - RES_BASE] += (*loadLink)->size;
		arm[(*loadLink)->altResMem] = TRUE;
	}

	nResourceTypes = 0;
	for (type = 0; type < NRESTYPES; type++)
		if (sizes[type])
			nResourceTypes++;
	nResourceSummaryLines = (nResourceTypes + NRES_SUMS_PER_LINE - 1) /
										NRES_SUMS_PER_LINE;

	for (nARMTypes = 0, type = 0; type < NARMTYPES; type++)
		nARMTypes += arm[type];
	nARMTypes--;	//	subtract one for "normal" memory

	/*	determine max size of window, in lines	*/
	maxLines = (wmgrPort->portRect.bottom - wmgrPort->portRect.top + 1) /
				  GetPointSize() - 3;
	maxResourceLines = maxLines - nResourceSummaryLines - NMEMORY_SUMMARY_LINES
								- nARMTypes;
	--maxResourceLines; 	//	allow one line for slop

	first = TRUE;
	loadLink = (LoadLink far**) Native(FFirstNode(&loadList));
	while (loadLink) {
		// create a window full of text
		*buf = 0;
		bp = buf;
		entriesOnLine = 0;

		while (loadLink) {
			if ((*loadLink)->type == RES_MEM) {
				lockedSize+=(*loadLink)->size;
				bp += sprintf(bp, "%uK ",
					(HandleSize((Handle) Native((*loadLink)->id)) + 1023) / 1024);

         } else {
				if (!(*loadLink)->altResMem &&
				    HandleLocked((*loadLink)->lData.data)) {
					*bp++ = '^';
				}
				if ((*loadLink)->lock == LOCKED) {
					lockedSize += (*loadLink)->size;
					*bp++ = '*';
				}
				if ((*loadLink)->altResMem)
					*bp++ = AltResMemDebugPrefix(loadLink);

				sprintf(bp, "%c%03d ",
					resSym[(*loadLink)->type - RES_BASE],
					(*loadLink)->id);
				bp += 5;
			}

			loadLink = (LoadLink far**) Native(FNextNode(Pseudo(loadLink)));

			// to avoid calling RTextSize on every entry, which is very slow,
			// we only call after the minimum # entries that will fit on a line
			if (++entriesOnLine > MIN_ENTRIES_LINE) {
				RTextSize(&rect, buf, DEBUGFONT, BIGWINDOW);
				entriesOnLine = 0;

				if ((rect.bottom - rect.top) / pointSize + 1 > maxResourceLines)
					break;
			}
		}

		// if there's more entries...
		if (loadLink)
			strcat(buf, " . . .");

		// add the footer
		if (first) {

			title = "Resources";
			for (type = 0, printCount = 0; type < NRESTYPES; type++) {
				if (sizes[type]) {
		         if (!(printCount % NRES_SUMS_PER_LINE))
		         	strcat(buf, "\r");
		         ++printCount;
					sprintf(buf + strlen(buf), "%4uK of %s    ",
		            (uint) ((sizes[type] + 1023L) / 1024L),
		            ResName(type + RES_BASE));
		   	}
		   }

		   strcat(buf, "\r");

			sprintf(buf + strlen(buf),
				" %4uK hunk:  %uK used  %uK free %uK locked\r",
				(uint) ((63L + hunkAvail) / 64),
				(uint) ((63L + (hunkAvail - FreeHunk())) / 64),
				(uint) ((63L + FreeHunk()) / 64),
				(uint) (lockedSize / 1024L));

			sprintf(buf + strlen(buf),
				" %5u heap:  %u used  %u free\r",
				heapAvail,
				heapAvail - FreeHeap(),
				FreeHeap());

			sprintf(
				buf + strlen(buf),
				"%u handles:  %u used  %u free\r",
				numHandles,
				numHandles - CountHandles(),
				CountHandles()
			);

			if (useAltResMem)
				AltResMemDebugSummary(buf + strlen(buf));

		} else {
			title = "Resources (cont.)";
		   strcat(buf, "\r");
			if (useAltResMem)
				AltResMemDebugKey(buf + strlen(buf));
		}


		DebugWindow(buf, title, BIGWINDOW, -1, -1, TRUE);
		if (dbgEvent.message == ESC)
			break;
		first = FALSE;

		maxResourceLines = maxLines - NARM_KEY_LINES;
		//	allow one line for slop
		--maxResourceLines;
	}
	LoseDebugBuf();
}


static uint near CountHandles()
{
   int * handle = (int *) handleBase;
   int   i,j;
   for (i=0,j=0;i < numHandles;++handle,++i)
      if ((*handle++ == 0) && (*handle == 0)) ++j;
   return (j);
}

global int CheckLoadLinks()
{
	LoadLink far * *scan;

	for
   (
	scan = (LoadLink far * *) Native(FFirstNode(&loadList));
	scan;
	scan = (LoadLink far * *) Native(FNextNode(Pseudo(scan)))
	)
	   {
      if ((*scan)->checkSum != 0x44)
         {
         return FALSE;
         }
      }
   return TRUE;
}

int
MakeObjectsStr(
	char*	dest,
   bool	showAddresses,
   Obj**	firstObj,
   int	nObjs)
{
	//	make a string containing the names of 'nObjs' objects starting with
	// 'firstObj'.  Include their addresses if 'showAddresses'.
	// Return the number of objects in the string (0 == no more objects to
	// display)

   Obj*		curObj = *firstObj;
   int*		cur;
   word**	nameProp;
   int		nPrintedObjs = 0;

   *dest = 0;

   if (!curObj)
   	curObj = (Obj*) handleBase;

   while (1) {
      //	find previous object by searching backwards in the heap for an OBJID
      cur = (int*) curObj;
      do
   	   cur--;
      while ((char*) cur > &bssEnd && *cur != OBJID);
      if ((char*) cur <= &bssEnd)
   	   break;
      curObj = (Obj*) cur;

      //	make sure this object is to be displayed and has a name property
      //	which is printable

      if (curObj->info & NODISPLAY)
      	continue;
      if (!(nameProp = GetPropAddr(curObj, s_name)))
      	continue;
      if (!IsPrintStr((char*) *nameProp))
      	continue;

      //	add this object's name (and address, if nec.) to the string
      //	precede with a '*' if object is dynamic
		sprintf(dest, "%s%s", curObj->info & CLONEBIT ? "*" : "",
			(char*) *nameProp);
      dest += strlen(dest);
		if (showAddresses) {
      	sprintf(dest, "@%x", curObj);
      	dest += strlen(dest);
      }
      strcat(dest, "  ");
      dest += 2;

      if (++nPrintedObjs >= nObjs)
      	break;
   }

   *firstObj = curObj;

	return nPrintedObjs;
}

static Obj*
FindObject(
	char*	name)
{
	//	find the object with 'name' or return NULL

   Obj*		curObj = (Obj*) &bssEnd;
   int*		cur;
   word**	nameProp;

   while (1) {
      //	find next object by searching in the heap for an OBJID
      cur = (int*) curObj;
      do
   	   cur++;
      while ((uchar*) cur < handleBase && *cur != OBJID);
      if ((uchar*) cur >= handleBase)
   	   return NULL;
      curObj = (Obj*) cur;

      //	if it has a name property and it's the one we're looking for,
      //	return it
      if ((nameProp = GetPropAddr(curObj, s_name)) &&
      	 !strcmp(*(char**) nameProp, name))
      	return curObj;
   }
}

#endif /* ifndef-else DEBUG */
