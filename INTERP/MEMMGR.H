// MEMMGR.H

#if !defined(MEMMGR_H)
#define MEMMGR_H

#if !defined(TYPES_H)
#include	"types.h"
#endif

#if !defined(LIST_H)
#include	"list.h"
#endif

#if defined(MPU68000)

/*
 * The Native and Pseudo functions convert (respectively) from
 *	a pseudo-machine virtual pointer to a true machine pointer, and
 *	from a true machine pointer to a pseudo-machine virtual pointer.
 */
strptr	Native();
memptr	NativeFar();
uword		Pseudo();

/* These macros define (in a machine-independent way, I hope!) access
 * to words in 8086 native format (low byte/high byte).
 */
word	GetWord();
word	GetWordP();
long	GetFarLongP();

#else

#define Native(x)		((strptr) (x))
#define NativeFar(x)	((ubyte _far *) (x))
#define Pseudo(addr)	((ObjID) (addr))

/* These are the 8086 versions.  Since the files are stored in 8086
 *	format (low byte first), nothing needs to be done.
 */
/* These access a given memory address (directly/through a pointer).
 */
#define	GetWord(w)			(w)
#define	GetWordP(p)			(*((word *) (p)))
#define	GetFarLongP(p)		(*((long _far *) (p)))

#endif

enum memFuncs {
	LARGESTPTR,
	FREEHEAP,
	LARGESTHANDLE,
	FREEHUNK,
	TOTALHUNK
};

// Free list structure for pointers
typedef struct {
	word				size;	// size of pointer
	struct block*	next;	// pointer to next free block in list
} block;

extern block*	freeList;
extern uint		heapAvail;
extern uint		hunkAvail;
extern memptr	heapBase;
extern memptr	handleBase;

void		InitMem(word maxHunkSize);
Handle	GetHandle(void);
void*		NeedPtr(uint);
void*		RNewPtr(uint);
void		ClearPtr(void *);
void		FillPtr(void *, ubyte);
void		DisposePtr(void *);
void*		ReallocatePtr(void *, uint);
uint		LargestPtr(void);
uint		FreeHeap(void);
uint		PtrSize(void *);
void		SaveHeap(void);
void		RestoreHeap(void);
void		RestartHeap(void);
Handle	NeedHandle(uint);
Handle	RNewHandle(uint);
void		ClearHandle(Handle);
void		FillHandle(Handle, ubyte);
void		DisposeHandle(Handle);
void*		ReallocateHandle(Handle, uint);
ulong		LargestHandle(void);
uint		FreeHunk(void);
uint		HandleSize(Handle);
int		HandleLocked(Handle);
void		LockHandle(Handle);
void		UnlockHandle(Handle);
void		CriticalHandle(Handle theHandle, bool critical);
void		PackHandles(void);
void*		CopyHandle(Handle, void *);
uint		StackLeft(void);
void*		hunkcpy(voidptr dst, farptr src, uint len);
int		CheckLoadLinks(void);

void		ShowHunks(void);
void		ShowHandles(void);
	 
#endif
