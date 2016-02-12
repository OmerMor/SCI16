// EVENT - Event manager routines

#include	"config.h"
#include	"driver.h"
#include	"errmsg.h"
#include	"event.h"
#include	"eventasm.h"
#include	"intrpt.h"
#include	"kernel.h"
#include	"memmgr.h"
#include	"mouse.h"
#include	"pmachine.h"
#include	"resource.h"
#include	"script.h"
#include	"selector.h"
#include	"start.h"
#include	"stdlib.h"
#include	"string.h"

/* Handle and pointer to keyboard driver.
 */
Handle	kbdHandle, joyHandle;
Hunkptr	keyboard, joystick;

static REventRecord* near bump(REventRecord *);

void
InitEvent(
	word	num)		/* init manager for num events */
{
	/* Setup the event queue.
	 */
	evQueue = (REventRecord *) NeedPtr(num * sizeof(REventRecord));
	evHead = evTail = evQueue;
	evQueueEnd = (evQueue + num);

	/* Load and initialize the keyboard driver.
	 */
	if (kbdDriver == NULL || (kbdHandle = LoadHandle(kbdDriver)) == NULL) {
		RAlert(E_NO_KBDDRV);
		exit(1);
		}
	else {
		LockHandle(kbdHandle);
		keyboard = *kbdHandle;
		KeyboardDriver(D_INIT);
		InstallServer(PollKeyboard, 1);
		}

	/* If there is a joystick, load and initialize the joystick driver.
	 */
	if (joyDriver != NULL && (joyHandle = LoadHandle(joyDriver)) != NULL) {
		LockHandle(joyHandle);
		joystick = *joyHandle;
		JoystickDriver(D_INIT);
		}
}

void
EndEvent()
{
	if (joyHandle)							  
		JoystickDriver(D_TERMINATE);
	DisposeServer(PollKeyboard);
	KeyboardDriver(D_TERMINATE);
	DisposeMouse();
}

bool
RGetNextEvent(	
	word				mask,
	REventRecord*	event)
{
	/* return next event to user */

	word ret = 0;
	REventRecord *found;

	if (joyHandle)
		PollJoystick();

	found = evHead;
	while (found != evTail){	/* scan all events in evQueue */
		if (found->type & mask){
			ret = 1;
			break;
		}
		found = bump(found);
	}

	if (ret){		/* give it to him and blank it out */
		memcpy((memptr) event, (memptr) found, sizeof(REventRecord));
		found->type = nullEvt;
		evHead = bump(evHead);
	} else {
		MakeNullEvent(event);	/* use his storage */
	}
	return(ret);
}

void
RFlushEvents(
	word	mask)
{
	// Flush all events specified by mask from buffer.

	REventRecord	event;

	while (RGetNextEvent(mask, &event))
		;

	if (mask & (keyDown | keyUp))
		FlushKeyboard();
}

bool
REventAvail(
	word				mask,
	REventRecord*	event)
{
	// return but don't remove

	word ret = 0;
	REventRecord *found;

	found = evHead;
	while (found != evTail){	/* scan all events in evQueue */
		if (found->type & mask){
			ret = 1;
			break;
		}
		found = bump(found);
	}

	/* a null REventRecord pointer says just return result */
	if (event){
		if (ret)
			memcpy((memptr) event, (memptr) found, sizeof(REventRecord));
		else
			MakeNullEvent(event);

	}

	return(ret);
}

bool
RStillDown()
{
	// look for any mouse ups
	return !REventAvail(mouseUp, 0);
}

void
RPostEvent(
	REventRecord*	event)
{
	// add event to evQueue at evTail, bump evHead if == evTail

	event->when = RTickCount();
	memcpy((memptr) evTail, (memptr) event, sizeof(REventRecord));
	evTail = bump(evTail);
	if (evTail == evHead)	/* throw away oldest */
		evHead = bump(evHead);
}

void
MakeNullEvent(
	REventRecord *event)
{
	//	give him current stuff

	event->type = 0;
	CurMouse(&(event->where));
	event->modifiers = GetModifiers();
}

void
KMapKeyToDir(
	word*	args)
{
	Obj				*SCIEvent;
	REventRecord	event;
	
	SCIEvent = (Obj *) Native(*(args+1));
	ObjToEvent(SCIEvent, &event);
	MapKeyToDir(&event);
	EventToObj(&event, SCIEvent);
	acc = Pseudo(SCIEvent);
}

REventRecord*
MapKeyToDir(
	REventRecord*	event)
{
	KeyboardDriver(INP_MAP, event);
	return event;
}

void
EventToObj(
	REventRecord*	evt,
	Obj				*evtObj)
{
	IndexedProp(evtObj, evType) = evt->type;
	IndexedProp(evtObj, evMod) = evt->modifiers;
	IndexedProp(evtObj, evMsg) = evt->message;
	IndexedProp(evtObj, evX) = evt->where.h;
	IndexedProp(evtObj, evY) = evt->where.v;
}

void
ObjToEvent(
	Obj				*evtObj,
	REventRecord*	evt)
{
	evt->type = IndexedProp(evtObj, evType);
	evt->modifiers = IndexedProp(evtObj, evMod);
	evt->message = IndexedProp(evtObj, evMsg);
	evt->where.h = IndexedProp(evtObj, evX);
	evt->where.v = IndexedProp(evtObj, evY);
}

void
KJoystick(
	word*	args)
{
	switch (arg(1))
		case JOY_RPT_INT:
			acc = JoystickDriver(JOY_RPT_INT, arg(2));
}

static REventRecord* near
bump(
	REventRecord*	ptr)
{
	//	move evQueue pointer to next slot

	if (++ptr == evQueueEnd)
		ptr = evQueue;
	return ptr;
}
