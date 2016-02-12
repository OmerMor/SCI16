/* TYPES.H
 */

#ifndef	TYPES_H
#define	TYPES_H

#ifdef	MPU68000
#define	far
#define	near
#endif

/* Shorthand for unsigned values.
 */
typedef	unsigned char		uchar;
typedef	unsigned int		uint;
typedef	unsigned long		ulong;

/* These should always be 8 bit values.
 */
typedef	char					byte;
typedef	uchar					ubyte;

/* These should always be 16 bit values.
 */
typedef	int					word;
typedef	uint					uword;

/* This type should be able to hold the number
 * of chars addressable by the machine.
 */
#ifndef _SIZE_T_DEFINED
typedef	uint					size_t;
#define _SIZE_T_DEFINED
#endif

// Boolean type
typedef	int					bool;

// Some pointer types
typedef	void					*voidptr;
typedef	char					*strptr;
typedef	ubyte					*memptr;

// A pointer to a function
typedef	void					(*fptr)(void);
typedef	bool					(*boolfptr)();

// far hunk stuff
typedef	void _far			*farptr;
typedef	ubyte _far			*Hunkptr;
typedef	uword _far			*Dictptr;
typedef	Hunkptr	    		*Handle;

// Pseudo-machine object (or other address) references
typedef	uword	ObjID;

#define	TRUE		1
#define	FALSE		0

#ifdef NULL
#undef NULL
#endif
#define	NULL		0
#define	LNULL		0L

#define	NULL_OBJ		((Obj *) 0)

#define	forever	for (;;)
#define	global

#define min(a,b) ((a)<(b) ? (a): (b))
#define max(a,b) ((a)>(b) ? (a): (b))

#endif

