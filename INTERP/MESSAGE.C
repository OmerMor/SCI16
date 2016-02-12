/****************************************************************************
 message.c		Mark Wilden, July 1991

 Access to message resources

 (c) Sierra On-Line, 1991
*****************************************************************************/

#include	"ctype.h"
#include	"errmsg.h"
#include	"kernel.h"
#include	"memmgr.h"
#include	"message.h"
#include	"mono.h"
#include "pmachine.h"
#include "resname.h"
#include	"resource.h"
#include	"stdio.h"
#include	"string.h"

////////////////////////////////////////////////////////////////////////////
//	message stack structures and functions

typedef struct {
	uchar	noun;
	uchar	verb;
	uchar	cond;
	uchar	seq;
} MsgEntry;

#define MaxMsgStack	10

typedef struct {
	uint		module;
	MsgEntry	entries[MaxMsgStack];
	int		sp;
} MsgStack;

static MsgStack stack;

static void			initStack(MsgStack* stack, uint module, uchar noun,
									uchar verb, uchar cond, uchar seq);
static void			push(MsgStack* stack, uchar noun, uchar verb, uchar cond,
									uchar seq);
static MsgEntry	top(MsgStack* stack);
static bool			pop(MsgStack* stack);
static void			incSeq(MsgStack* stack);

static uint	curModule;
static uint	curNoun;
static uint	curVerb;
static uint	curCond;
static uint	curSeq;
static Handle xlateHandle;

///////////////////////////////////////////////////////////////////////////////

static void			pushStack(void);
static void			popStack(void);

///////////////////////////////////////////////////////////////////////////////

#define MSG_GET			0
#define MSG_NEXT			1
#define MSG_SIZE			2
#define MSG_GETREFNOUN	3
#define MSG_GETREFVERB	4
#define MSG_GETREFCASE	5
#define MSG_PUSH			6
#define MSG_POP			7
#define MSG_GETKEY		8

static void			get(uint module, uchar noun, uchar verb, uchar cond,
								  uchar seq, strptr buffer);
static bool			getRefValues(uint module, uchar noun, uchar verb, uchar cond,
								  uchar seq, uchar* refNoun, uchar* refVerb,
								  uchar* refCond);
static void			getNext(strptr buffer);
static void			showXlate(void);
static void			getSize(uint module, uchar noun, uchar verb, uchar cond,
									uchar seq);
static char far*	find(MsgStack* stack, IndexEntry far** indexEntry, bool deep);
static char far*	findXlate(void);
static void			massageText(char far* dst, char far* src);

////////////////////////////////////////////////////////////////////////////

void
KMessage(
	word* args)
{
	switch (arg(1)) {
		case MSG_GET:
			get(	(uint) arg(2),									// module
					(uchar) arg(3),								// noun
					(uchar) arg(4),								// verb
					(uchar) arg(5),								// case
					(uchar) arg(6),								// sequence
					argCount < 7 ? NULL : Native(arg(7))	// buffer
				);
			break;
		
		case MSG_NEXT:	
			getNext(argCount < 2 ? NULL : Native(arg(2)));	//	buffer
			break;
		
		case MSG_SIZE:
			getSize(	(uint) arg(2),		// module
						(uchar) arg(3),	// noun
						(uchar) arg(4),	// verb
						(uchar) arg(5),	// case
						(uchar) arg(6)		// sequence
					 );
			break;
		
		case MSG_GETREFNOUN:
		case MSG_GETREFVERB:
		case MSG_GETREFCASE:
		{
			uchar	refNoun;
			uchar	refVerb;
			uchar	refCond;
			bool rc = getRefValues(	(uint) arg(2),		// module
											(uchar) arg(3),	// noun
											(uchar) arg(4),	// verb
											(uchar) arg(5),	// case
											(uchar) arg(6),	// sequence
											&refNoun,
											&refVerb,
											&refCond
							 );
			if (!rc)
				acc = -1;
			else 
				acc = arg(1) == MSG_GETREFNOUN ? refNoun :
						arg(1) == MSG_GETREFVERB ? refVerb :
						                           refCond;
			break;
		}

		case MSG_PUSH:
			pushStack();
			break;
			
		case MSG_POP:
			popStack();
			break;
		
		case MSG_GETKEY:
		{
			int* ip = (int*) arg(2);
			ip[0] = curModule;
			ip[1] = curNoun;
			ip[2] = curVerb;
			ip[3] = curCond;
			ip[4] = curSeq;
			break;
		}
		
		default:
			Panic(E_MESSAGE, arg(1));
	}
}

static void
get(
	uint		module,
	uchar		noun,
	uchar		verb,
	uchar		cond,
	uchar		seq,
	strptr	buffer)
{
	//	sets current message stack parameters and gets message associated with
	//	them
	//	if found, acc = talker number and if buffer supplied, copies the message
	//		to the buffer minus stage directions
	//	else acc = 0 and if buffer supplied, copies an error message to it

	initStack(&stack, module, noun, verb, cond, seq);
	
	getNext(buffer);
}

static void
getNext(
	strptr	buffer)
{
	//	finds the next message in the sequence on the stack
	//	if found, acc = talker number and if buffer supplied, copies the message
	//		to the buffer minus stage directions and increments current seq num
	//	else acc = 0 and if buffer supplied, copies an error message to it

	IndexEntry far*	indexEntry;
	MsgStack				saveStack;
	char far*			msg;
	MsgEntry				entry;
	
	//	save the stack if this is just a query
	if (!buffer)
		saveStack = stack;

	msg = find(&stack, &indexEntry, TRUE);

	if (!msg) {
		if (buffer) {
			entry = top(&stack);
			sprintf(buffer, "Msg %d: %d %d %d %d not found",
									stack.module, entry.noun, entry.verb, entry.cond,
									entry.seq);
		}
		acc = 0;

	} else {
		if (buffer) {
			massageText((char far *)buffer, msg);
			entry = top(&stack);
			curModule = stack.module;
			curNoun = entry.noun;
			curVerb = entry.verb;
			curCond = entry.cond;
			curSeq = entry.seq;
			incSeq(&stack);
			showXlate();
		}
		acc = indexEntry->talker;
	}
	
	//	if this is just a query, restore stack
	if (!buffer)
		stack = saveStack;
}

static void
showXlate() {
	int len;
	char far* cp;
	char far* xmsg;
	Handle bufferHandle;

	if (xlate) {
		if (xmsg = findXlate()) {
			for (len = 0, cp = xmsg; *cp; cp++, len++)
				;
			//	add one for trailing null
			len++;
			ResLock(RES_XLATE, curModule, TRUE);
			LockHandle(xlateHandle);
			bufferHandle = ResLoad(RES_MEM,len);
			massageText((char far *)*bufferHandle, xmsg);
			UnlockHandle(xlateHandle);
			ResLock(RES_XLATE, curModule, FALSE);
		} else {
			bufferHandle = ResLoad(RES_MEM,40);
			massageText((char far *)*bufferHandle, (char far *)"(translation not found)");
		}
		MonoStr((char far *)*bufferHandle);
		ResUnLoad(RES_MEM,(uword) bufferHandle);
	}
}

static void
getSize(
	uint		module,
	uchar		noun,
	uchar		verb,
	uchar		cond,
	uchar		seq)
{
	//	return size of buffer needed to hold the message, including trailing
	//	NULL, or 0 if message not found
	//	stage directions are included in the size

	MsgStack		stack;
	char far*	cp;

	//	use a local stack, since no need to save info across calls
	initStack(&stack, module, noun, verb, cond, seq);

	cp = find(&stack, NULL, TRUE);

	if (!cp)
		acc = 0;

	else {
		int len;
		for (len = 0; *cp; cp++, len++)
			;
		//	add one for trailing null
		acc = len + 1;
	}
}

static bool
getRefValues(
	uint		module,
	uchar		noun,
	uchar		verb,
	uchar		cond,
	uchar		seq,
	uchar*	refNoun,
	uchar*	refVerb,
	uchar*	refCond)
{
	//	set reference values for this message, returning FALSE if message not
	//	found.

	IndexEntry far*	indexEntry;
	MsgStack				stack;

	//	use a local stack, since no need to save info across calls
	initStack(&stack, module, noun, verb, cond, seq);

	if (!find(&stack, &indexEntry, FALSE))
		return FALSE;

	*refNoun = indexEntry->refNoun;
	*refVerb = indexEntry->refVerb;
	*refCond = indexEntry->refCond;

	return TRUE;
}

static char far*
find(
	MsgStack*			stack,
	IndexEntry far* *	indexEntry,
	bool					deep)
{
	//	finds a message and returns far pointer to it and to its index entry
	//	if found, else NULL
	//	'deep' means whether reference chain is followed

	register uint		i;
	MsgData far*		data = (MsgData far*) *ResLoad(RES_MSG, stack->module);
	uint					count = GetWord(data->count);
	IndexEntry far*	ip;
	
	if (data->version < MessageMajorVersion)
		Panic(E_BADMSGVERSION);
	
	while (1) {
		//	search for message on top of stack
		MsgEntry entry;
		entry = top(stack);
		for (i = 0, ip = data->entries; i < count; i++, ip++)
			if (ip->noun == entry.noun &&
				 ip->verb == entry.verb &&
				 ip->cond == entry.cond &&
				 ip->seq == entry.seq
				)
					break;
		
		//	if wasn't found, try to pop; otherwise return failure
		if (i == count)
			if (deep && pop(stack))
				continue;
			else {
				if (indexEntry)
					*indexEntry = NULL;
				return NULL;
			}
		
		//	if it's a reference, push it onto the stack and go back around
		if (deep && (ip->refNoun || ip->refVerb || ip->refCond)) {
			incSeq(stack);
			push(stack, ip->refNoun, ip->refVerb, ip->refCond, 1);
			continue;
		}

		//	otherwise, return the message
		if (indexEntry)
			*indexEntry = ip;
		return (char far*) data + GetWord(ip->offset);
	}
}

static char far*
findXlate()
{
	//	finds a message translation and returns far pointer to it
	//	if found, else NULL
	//	'deep' means whether reference chain is followed

	register uint		i;
	MsgData far*		data = (MsgData far*) *(xlateHandle = ResLoad(RES_XLATE, curModule));
	uint					count = GetWord(data->count);
	IndexEntry far*	ip;
	
	if (data->version < MessageMajorVersion)
		Panic(E_BADMSGVERSION);
	
	//	search for message
	for (i = 0, ip = data->entries; i < count; i++, ip++)
		if (ip->noun == (uchar)curNoun &&
			 ip->verb == (uchar)curVerb &&
			 ip->cond == (uchar)curCond &&
			 ip->seq  == (uchar)curSeq
			)
				break;
	
	//	if wasn't found, return failure
	if (i == count)
		return NULL;
	
	//	otherwise, return the message
	return (char far*) data + GetWord(ip->offset);
}

static void
massageText(
	char far*	dst,
	char far*	src)
{
	//	copies src into dst, deleting stage directions, which are phrases
	//	in caps enclosed by parens; and interpreting escape sequences of
	//	the form \xx, where x is a hex digit

	static char	hexDigits[] = "01234567890ABCDEF";

	char far*	dp = dst;

	for (; *src; src++) {
		if (*src == '\\') {
			char* cp;
			int	val = 0;
			char	c;
			
			src++;

			//	check first digit
			c = _toupper(*src);
			if (!(cp = strchr(hexDigits, c))) { 
				*dp++ = *src;
				continue;
			} else {
				val = val * 16 + cp - hexDigits;
				src++;
			}
			
			//	check second digit
			c = _toupper(*src);
			if (!(cp = strchr(hexDigits, c))) {
				src--;
				continue;
			} else
				val = val * 16 + cp - hexDigits;

			*dp++ = (uchar) val;

		} else if (*src == '(') {
			/*	find end of this stage direction (if it is) and set src to it*/
			char far* end;
			for (end = src; *end; end++)
				if (*end == ')') {
					/*	found end of stage direction:
						strip trailing whitespace, then exit	*/
					end++;
					while (*end && (*end == ' ' || *end == '\n' || *end == '\r'))
						end++;
					src = --end;
					break;

				} else if (*end >= 'a' && *end <= 'z' ||
							  *end >= '0' && *end <= '9') {
					/*	it's not a stage direction	*/
					*dp++ = *src;
					break;
				}

		} else
			*dp++ = *src;
	}
	*dp = '\0';
}

///////////////////////////////////////////////////////////////////////////
//	stack functions

static void
initStack(
	MsgStack*	stack,
	uint			module,
	uchar			noun,
	uchar			verb,
	uchar			cond,
	uchar			seq)
{
	//	start a new stack

	stack->module = module;
	stack->sp = -1;
	
	push(stack, noun, verb, cond, seq);
}

static void
push(
	MsgStack*	stack,
	uchar			noun,
	uchar			verb,
	uchar			cond,
	uchar			seq)
{
	//	push a message onto stack

	if (++stack->sp >= MaxMsgStack)
		Panic(E_MSGSTACKOVERFLOW, MaxMsgStack);
	stack->entries[stack->sp].noun	= noun;
	stack->entries[stack->sp].verb	= verb;
	stack->entries[stack->sp].cond	= cond;
	stack->entries[stack->sp].seq		= seq;
}

static MsgEntry
top(
	MsgStack*	stack)
{
	//	return the message on the top of stack, but don't pop it off

	MsgEntry entry;
	entry.noun	= stack->entries[stack->sp].noun;
	entry.verb	= stack->entries[stack->sp].verb;
	entry.cond	= stack->entries[stack->sp].cond;
	entry.seq	= stack->entries[stack->sp].seq;
	return entry;
}

static bool
pop(
	MsgStack*	stack)
{
	//	throw away top of stack

	return --stack->sp >= 0;
}

static void
incSeq(
	MsgStack*	stack)
{
	//	increment the sequence number of the entry on the top of stack

	stack->entries[stack->sp].seq++;
}

///////////////////////////////////////////////////////////////////////////
//	stack stack functions

#define MaxStackStack	5

static MsgStack*	stackStack[MaxStackStack];
static int			stackSp = -1;

static void
pushStack()
{
	//	save current stack on stack stack

	MsgStack* s;

	if (++stackSp >= MaxStackStack)
		Panic(E_MSGSTACKSTACKOVERFLOW, MaxStackStack);

	s = NeedPtr(sizeof(MsgStack));
	*s = stack;
	stackStack[stackSp] = s;
}

static void
popStack()
{
	//	restore previous saved stack
	
	MsgStack* s;

	if (stackSp < 0)
		Panic(E_MSGSTACKSTACKUNDERFLOW);
		
	s = stackStack[stackSp--];
	stack = *s;
	DisposePtr(s);
}
