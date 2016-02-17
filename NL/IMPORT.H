/* Project: SCI Interpreter
 *
 * Module:	import.h
 *
 * Author:	Jeff Stephenson
 *
 * Definitions for functions imported from the kernel.
 */


#ifndef	IMPORT_H

#define	IMPORT_H


enum	importFunc {
	BLKNEW,
	ALERT,
	STRPTRGET,
	RESLOAD,
	RESLOCK,
	RESDATA,
	STRPTRDUP,
	INVOKEMETHOD,
	BLK_FREE,
	SETPROPERTY,
	GETPROPERTY,
	STRNCPY,
	SPRINTF,
	STRBCAT,
	STRCMP,
	STRCPY,
	STRLWR,
	STRTRN,
	ITOA,
	MODULEFIND,
	SEQNEW,
	SEQNEXT,
	};


#define	BlkNew(size) \
				(ID) (ift[BLKNEW].function)(size)

#define	BlkFree(id) \
				(void) (ift[BLK_FREE].function)(id)

#define	Alert(msg) \
				(bool) (ift[ALERT].function)(msg)

#define	StrPtrGet(id) \
				(strptr) (ift[STRPTRGET].function)(id)

#define	ResLoad(resType, resID) \
				(ID) (ift[RESLOAD].function)(resType, resID)

#define	ResLock(resType, resID, lock) \
				(void) (ift[RESLOCK].function)(resType, resID, lock)

#define	ResData(res) \
				(memptr) (ift[RESDATA].function)(res)

#define	StrPtrDup(str) \
				(ID) (ift[STRPTRDUP].function)(str)

//#define	InvokeMethod(obj, selector, n, ...) \
//				(int) (ift[INVOKEMETHOD].function)(obj, selector, n, ...)

#define	SetProperty(obj, selector, value) \
				(void) (ift[SETPROPERTY].function)(obj, selector, value)

#define	GetProperty(obj, selector) \
				(int) (ift[GETPROPERTY].function)(obj, selector)

#define	strncpy(dest, src, n) \
				(strptr) (ift[STRNCPY].function)(dest, src, n)

//#define	sprintf(str, format, ...) \
//				(int) (ift[SPRINTF].function)(str, format, ...)

#define	strbcat(dest, src, count) \
				(strptr) (ift[STRBCAT].function)(dest, src, count)

#define	strtrn(ins, inpatt, outpatt, outs) \
				(bool) (ift[STRTRN].function)(ins, inpatt, outpatt, outs)

#define	itoa(num, str, radix) \
				(strptr) (ift[ITOA].function)(num, str, radix)

#define	ModuleFind(n) \
				(ID) (ift[MODULEFIND].function)(n)

#define	SeqNew(list, type) \
				(ID) (ift[SEQNEW].function)(list, type)

#define	SeqNext(seq) \
				(ID) (ift[SEQNEXT].function)(seq)



#endif
