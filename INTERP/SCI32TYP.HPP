// types.hpp	IBM	5/25/93
//		platform-specific typedefs

#ifndef TYPES_HPP
#define TYPES_HPP

#ifndef _STDDEF_H_INCLUDED
#include <stddef.h>
#endif

//	abbreviations
typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned int		uint;
typedef unsigned long	ulong;

// Boolean
typedef int	Bool;

//	data types for external data (files or OS data structures)
//	each contain the number of bits indicated
typedef short	Int16;
typedef ushort	UInt16;
typedef int		Int32;
typedef int		UInt32;

//	data types for interfacing to the SCI language, in which all values
//	are 16 bits
typedef Int16		SCIWord;
typedef UInt16		SCIUWord;
typedef SCIUWord	Selector;
typedef SCIWord	Property;
typedef SCIWord	Acc;

////////////////////////////////////////////////////////////////////////
//	these are soon to disappear or change

typedef void		AlertProc(char*);

#endif

