//	sol.hpp	5/25/93
//		standard header for all Sierra On-Line C++ programs

#ifndef SOL_HPP
#define SOL_HPP

//	turn off WATCOM's "assignment found in boolean" warning: 'if (a = b())'
#pragma warning 391	9

// turn off WATCOM's "must look ahead ..a declaration/type or an expression"
#pragma warning 604  9
#pragma warning 595  9

// turn off WATCOM's warning about truncation when assigning ints to shorter
#pragma warning 389 9

void	Panic(char*, ...);
#define assert(expr) \
	if (!(expr)) Panic("Assertion failed: %s: File %s(%d) ", \
		#expr, __FILE__, __LINE__);

#include "types.hpp"

const Bool	True = 1;
const Bool	False = 0;

#include "memid.hpp"

template<class T, class S>
inline S max(S	a, T b)
{
	return a > b ? a : b;
}

template<class T, class S>
inline S min(S	a, T b)
{
	return a < b ? a : b;
}

//	checks for success, and aborts if fails
#define New		new(newFailedStr, __FILE__,  __LINE__)
extern char 	newFailedStr[];
void*				operator new(size_t size, char* fmt, ...);

#endif
