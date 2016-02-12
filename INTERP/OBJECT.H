#if !defined(OBJECT_H)
#define OBJECT_H

#if !defined(TYPES_H)
#include "types.h"
#endif

#if !defined(SCRIPT_H)
#include "script.h"
#endif

#define	OBJID		(uword) 0x1234
#define	OBJNUM	(uword) 0xffff

// This structure defines the 'RootObj', the basic properties of all objects.
//	Resource fields must be uwords, because size of actual pointers is 
//	different for each platform.
typedef struct {
	uword		objID;			// object identifier (set to OBJID)
	uword		size;				// number of properties in the object
	uword		propDict;		// offset of property dictionary in hunk resource
	uword		methDict;		// offset of method dictionary in hunk resource
	ObjID		classScript;	// pointer to script node of object's class
	ObjID		script;			// pointer to script node for the object
	ObjID		super;			// pointer to super class of object
	uword		info;				// bit-mapped information
} Obj;

#define	IndexedPropAddr(object, prop)		(((word *) object) + objOfs[prop])
#define	IndexedProp(object, prop)			(*IndexedPropAddr(object, prop))

//	format of classTbl resource
typedef struct {
	ObjID		obj;				//	pointer to Obj
	uword		scriptNum;		//	script number
} ClassEntry;

extern ClassEntry*	classTbl;
extern int				objOfs[];

#ifdef	DEBUG
extern uword	sendStack[];
extern uword*	ssPtr;
extern uword	ssEnd;
#endif

int		GetProperty(Obj*, uint);
void		SetProperty(Obj*, uint, int);
void*		GetPropAddr(Obj*, uint);
int		InvokeMethod(Obj*, int, int, ...);
memptr	CopyScript(Handle, Script*, bool);
bool		IsObject(Obj*);
void		LoadPropOffsets(void);
bool		RespondsTo(Obj*, uint);
Obj*		Clone(Obj*);
void		DisposeClone(Obj*);
void		LoadClassTbl(void);

#endif
