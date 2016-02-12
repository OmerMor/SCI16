//	object.c

#include "debug.h"
#include "errmsg.h"
#include "pmachine.h"
#include "resource.h"
#include "script.h"
#include "selector.h"
#include "string.h"

int	objOfs[OBJOFSSIZE];

static void near CheckObject(Obj* obj);

Obj*
Clone(
	Obj*	obj)
{
	//	return pointer to copy of an object or class

   Obj*		newObj;
   word		size;
   Script*	sp;

	//	is 'obj' an object?
   CheckObject(obj);

   //	get memory and copy into it
   size = obj->size * 2;
	newObj = NeedPtr(size);
   memcpy(newObj, obj, size);

   //	if we're copying a class, set the new object's super to the class
   //	and turn off its class bit
   if (obj->info & CLASSBIT) {
		newObj->super = Pseudo(obj);
	   newObj->info &= ~CLASSBIT;
   }

   //	mark the object as cloned
   newObj->info |= CLONEBIT;

   //	increment script's reference count
   sp = (Script*) Native(newObj->script);
   ++sp->clones;

   return newObj;
}

void
DisposeClone(
	Obj*	obj)
{
	//	dispose of a clone of a class

	//	is 'obj' an object?
	CheckObject(obj);

   //	clear the object ID, decrement the script's reference count,
   //	and free the object's memory
   if ((obj->info & (CLONEBIT | NODISPOSE)) == CLONEBIT) {
   	Script* sp = (Script*) Native(obj->script);
   	obj->objID = 0;
      sp->clones--;
      DisposePtr(obj);
   }
}

bool
IsObject(
	Obj*	obj)
{
	//	a pointer points to an object if it's non-NULL, not odd, and
   //	its objID field is OBJID

   //	candidate for inlining

	return obj && !((word) obj & 1) && obj->objID == OBJID;
}

bool
RespondsTo(
	Obj*	obj,
	uint	selector)
{
	//	return whether 'selector' is a property or method of 'obj' or its
	//	superclasses

	word far*	methodDict;
   int			nMethods;

	//	is 'obj' an object?
   CheckObject(obj);

	//	is 'selector' a property?
	if (GetPropAddr(obj, selector))
		return TRUE;

	//	search the method dictionary hierarchy
   do {
   	Script* sp = (Script*) Native(obj->script);
	   methodDict = (word far*) (*sp->hunk + obj->methDict);
      for (nMethods = *methodDict++; nMethods--; methodDict += 2)
   	   if (*methodDict == selector)
      	   return TRUE;
      obj = (Obj*) Native(obj->super);
	} while (obj);

	return FALSE;
}

void
LoadPropOffsets()
{
	// Load the offsets to indexed object properties from a file.

	int			i;
	uword	_far*	op;

	op = (uword far *) *ResLoad(RES_VOCAB, PROPOFS_VOCAB);

	// Read and store each offset.
	for (i = 0 ; i < OBJOFSSIZE ; ++i)
		objOfs[i] = *op++;
}

void
LoadClassTbl()
{
	classTbl =
		(ClassEntry*) CopyHandle(ResLoad(RES_VOCAB, CLASSTBL_VOCAB), NULL);
	numClasses = PtrSize(classTbl) / sizeof(*classTbl);
}

static void near
CheckObject(
	Obj*	obj)
{
	if (!IsObject(obj))
		PError(thisIP, pmsp, E_NOT_OBJECT, (uint) obj, 0);
}
