// script.c
// 	routines to handle loading & unloading script modules.

/*	The hunk resource has the following format:

		------------------------------
		word: offset of beginning of fixups
		------------------------------
		word: address of script node
		------------------------------
		word: boolean: script has associated far text
		------------------------------
		word: length of dispatch table
		------------------------------
		dispatch table
		------------------------------
		code
		------------------------------
		word: number of fixups
		------------------------------
		fixups
		------------------------------

	The heap resource has the following format:

		------------------------------
		word: offset of beginning of fixups
		------------------------------
		word: number of variables
		------------------------------
		variables
		------------------------------
		objects
		classes
		meta-strings
		near strings
		------------------------------
		word: number of fixups
		------------------------------
		fixups
		------------------------------
*/

#include	"debug.h"
#include	"errmsg.h"
#include	"kernel.h"
#include	"pmachine.h"
#include	"resource.h"
#include	"selector.h"

static void near		InitHeapRes(Handle h, Script *sp);
static void	near		InitHunkRes(Handle h, Script *sp);
static void near		InitObj(Obj *op, Script *sp);
static void near		DoFixups(uword _far *fixTbl, ubyte _far *fixBase,
								ubyte _far *fixOfs, int sign);
static void	near		TossScriptObjects(Script *sp);
static Script* near	FindScript(int n);
static void near		TossScript(Script *sp, bool checkClones);
static void near		TossScriptClasses(uint n);
static Obj*	near		GetClass(int n);

void
InitScripts()
{
	// Initialize the list of loaded scripts.

	InitList(&scriptList);
}

Script*
ScriptPtr(
	int n)
{
	// Return a pointer to the node for script n.  Load the script if necessary.

	Script*	sp;

	if ((sp = FindScript(n)) == NULL)
		sp = LoadScript(n);

	return sp;
}

Script*
LoadScript(
	int n)
{
	// Load script number n in hunk space, copy it to the heap, allocate and
	// clear its local variables, and return a pointer to its script node.

	Script*	sp;
	Handle	heap;
	Handle	hunk;

	// Allocate and link a new node into the list.
	sp = (Script *) NeedPtr(sizeof(Script));
	ClearPtr(sp);
	AddToFront((List *) &scriptList, Pseudo(sp), n);

	// Load the heap and hunk resources.
	heap = ResLoad(RES_HEAP, n);
	hunk = ResLoad(RES_SCRIPT, n);
	ResLock(RES_SCRIPT, n, TRUE);
	InitHeapRes(heap, sp);
	InitHunkRes(hunk, sp);

	return sp;
}

void
ReloadScript(
	Script*	sp)
{
	uint		n;
	Handle	hunk;

	n = ScriptNumber(sp);
	hunk = ResLoad(RES_SCRIPT, n);
	ResLock(RES_SCRIPT, n, TRUE);
	InitHunkRes(hunk, sp);
}

static void near
InitHeapRes(
	Handle	h,
	Script*	sp)
{
	HeapRes _far*	heapRes = (HeapRes _far*) *h;
	HeapRes*			heap;
	uint				heapLen;
	Obj*				op;

	// The first word of the heap resource is the offset to the fixup tables
	// for the resource (which are at the end of the resource).  This is thus
	// the length of the heap resource.  Allocate the space for the heap
	// component and copy it into the space.
	heapLen = heapRes->fixOfs;
	heap = (HeapRes*) NeedPtr(heapLen);
	sp->heap = (HeapRes*) hunkcpy(heap, heapRes, heapLen);
	sp->vars = &heap->vars;

	// Do the necessary fixups on the portion loaded into heap.
	DoFixups((uword _far*) ((char _far*) heapRes + heapLen),
		(ubyte _far*) heap, (ubyte _far*) heap, 1);
	
	// Now initialize the objects in this heap resource.
	for (op = (Obj*) (&heap->vars + heap->numVars);
		  op->objID == OBJID;
		  op = (Obj*) ((uword*) op + op->size))
		InitObj(op, sp);
}

static void near
InitObj(
	Obj*		op,
	Script*	sp)
{
	Obj*		cp;

	// Set the address of the object's superclass (the compiler puts the
	// class number of the superclass in the -super- property)
	op->super = Pseudo(GetClass(op->super));

	if (op->script == OBJNUM) {
		// An object's -classScript- points to its superclass's script.
		cp = (Obj *) Native(op->super);
		op->propDict = cp->propDict;
		op->classScript = cp->script;

	} else {
		// If the object is a class, set its address in the class table (the
		// class number is stored in the -script- property by the compiler).
		classTbl[op->script].obj = Pseudo(op);
		op->classScript = Pseudo(sp);
	}

	// Set the object's '-script-' property to the address of the script
	// node being loaded.
	op->script = Pseudo(sp);
}

static void near
InitHunkRes(
	Handle	h,
	Script*	sp)
{
	HunkRes _far	*hunk;
	uword _far		*fp;

	sp->hunk = h;
	hunk = (HunkRes _far *) *sp->hunk;

	// Point to the script node for this hunk.
	hunk->script = Pseudo(sp);

	// Do the fixups in the hunk resource.
	fp = (uword _far *) (((ubyte _far *) hunk) + hunk->fixOfs);
	DoFixups(fp, (ubyte _far*) hunk, (ubyte _far*) sp->heap, 1);

	if (hunk->farText)
		ResLoad(RES_TEXT, ScriptNumber(sp));  
}

static void near
DoFixups(
	uword _far*	fixTbl,
	ubyte _far*	fixBase,
	ubyte _far*	fixOfs,
	int			sign)
{
	uint			numFix;
	uword _far*	hp;

	for (numFix = *fixTbl++; numFix-- ; ) {
		// get pointer to word to be fixed up
		hp = (uword _far *) (fixBase + *fixTbl++);
		*hp = sign > 0 ? Pseudo(*hp + fixOfs) :
			(ObjID) ((long) *hp - (long) fixOfs);
	}
}

void
DisposeAllScripts()
{
	// Dispose of the entire script list (for restart game).

	Script* sp;

	for (sp = (Script*) Native(FirstNode(&scriptList));
		  !EmptyList((List *) &scriptList);
		  sp = (Script*) Native(FirstNode(&scriptList)))
		TossScript(sp, FALSE);
}

void
DisposeScript(
	int n)
{
	// Remove script n from the active script list and deallocate the space
	// taken by its code and variables.

	Script	*sp;

	if ((sp = FindScript(n)) != NULL)
		TossScript(sp, TRUE);
}

static void near
TossScript(
	Script*	sp,
	bool		checkClones)
{
	int	n = ScriptNumber(sp);

	// Undo the fixups in the hunk resource and then unlock it.
	ResetHunk((HunkRes _far *) *sp->hunk);
	ResLock(RES_SCRIPT, n, FALSE);

	// Dispose the heap resource.
	TossScriptClasses(n);
	TossScriptObjects(sp);
	if (sp->heap)
		DisposePtr(sp->heap);
	if (checkClones && (sp->clones))
		PError(thisIP, pmsp, E_LEFT_CLONE, n, 0);
	DeleteNode((List *) &scriptList, Pseudo(sp));
	DisposePtr(sp);
}

void
ResetHunk(
	HunkRes _far*	hunk)
{
	Script*		sp;
	uword _far*	fp;

	sp = (Script *) Native(hunk->script);
	if (hunk->script) {
		hunk->script = 0;
		fp = (uword _far*) (((ubyte _far*) hunk) + hunk->fixOfs);
		DoFixups(fp, (ubyte _far*) hunk, (ubyte _far*) sp->heap, -1);
	}
}

static Script* near
FindScript(
	int n)
{
	// Return a pointer to the node for script n if it is in the script list,
	// or NULL if it is not in the list.

	return (Script *) Native(FindKey(&scriptList, n));
}

static Obj* near
GetClass(
	int n)
{
	if (n == -1)
		return NULL;

	if (!classTbl[n].obj) {
		LoadScript(classTbl[n].scriptNum);
		if (!classTbl[n].obj)
			PError(thisIP, pmsp, E_LOAD_CLASS, n, 0);
	}

	return (Obj*) Native(classTbl[n].obj);
}

static void near
TossScriptClasses(
	uint n)
{
	// Remove all classes belonging to script number n from the class table.

	uint	i;

	for (i = 0 ; i < numClasses ; ++i)
		if (classTbl[i].scriptNum == n)
			classTbl[i].obj = 0;
}

static void near
TossScriptObjects(
	Script*	sp)
{
	// Scan through the script unmarking the objects.

	Obj*		op;

	for (op = (Obj*) (&sp->heap->vars + sp->heap->numVars);
		  op->objID == OBJID;
		  op = (Obj*) ((uword*) op + op->size))
		op->objID = 0;
}

void
KDisposeScript(
	kArgs args)
{
   /* allow for return code calculation on disposescript */
   /* This prevents return                               */
   /*       (DisposeScript self)                         */
   /*       (return value)                               */
   /* from getting "executing in disposed script" error  */
   /* by using                                           */
   /*       (DisposeScript self value)                   */

	if (argCount == 2)
		acc = arg(2);
	DisposeScript(arg(1));
}

