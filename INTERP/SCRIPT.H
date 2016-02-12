// SCRIPT.H

#if !defined(SCRIPT_H)
#define SCRIPT_H

#if !defined(MEMMGR_H)
#include	"memmgr.h"
#endif

#if !defined(LIST_H)
#include	"list.h"
#endif

typedef struct {
	uword		fixOfs;		// offset to fixup table
	uword		numVars;		// number of variables
	uword		vars;			// beginning of variables
} HeapRes;

typedef struct {
	Node		link;		// list header (list node key is script number)
	HeapRes*	heap;		// pointer to data in heap for script
	Handle	hunk;		// handle pointer to hunkptr for data in hunk for script
	uword*	vars;		// pointer to variables for this script
   int      clones;	// number of clones spawned from this script
} Script;

typedef struct	{
	int		type;		// type of hunk
	int		size;		// size of hunk
	byte		data;		// hunk data follows
} Hunk;

typedef struct	{
	uword		fixOfs;		// offset to fixup table
	uword		script;		// pointer to script node in heap
	uword		farText;		// does this resource have far text?
	uword		numDisp;		// number of dispatch table entries
	uword		dispTbl;		// beginning of dispatch table
} HunkRes;

// Scripts with numbers higher than 900 are system scripts, as is script
// number 255.
#define	IsSysScript(n)		((n) >= 900 || (n) == 255)
#define	ScriptNumber(sp)	((sp)->link.key)

extern List	scriptList;

void		InitScripts(void);
Script*	ScriptPtr(int);
Script*	LoadScript(int);
void		ReloadScript(Script *sp);
void		DisposeScript(int);
void		ResetHunk(HunkRes _far *hunk);
void		DisposeAllScripts(void);

#endif

