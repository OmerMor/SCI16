//	time.cpp

#include <time.h>

#include	"sol.hpp"

#include "kernel.hpp"
#include "sci.hpp"
#include "time.hpp"

ulong	sysTicks;

ulong
RTickCount()
{
	return sysTicks;
}

uint
SysTime(int func)
{
	struct tm*	newtime;
	time_t		long_time;
	int			fmtTime;

	time(&long_time);
	newtime = localtime(&long_time);

	switch (func) {

		case 1:
			//return packed time
			//HHHH|MMMM|MMSS|SSSS - (hour 1-12)
			fmtTime = newtime->tm_hour;
			if (fmtTime == 0)
				fmtTime = 12;
			if (fmtTime > 12)
				fmtTime -= 12;
			fmtTime = (fmtTime << 12) |
						 (newtime->tm_min << 6) |
						 (newtime->tm_sec);
			break;

		case 2:
			//return packed time
			//HHHH|HMMM|MMMS|SSSS - (hour 0-24)
			//* note loss of SECOND resolution in this form
			fmtTime = (newtime->tm_hour << 11) |
						 (newtime->tm_min << 5) |
						 (newtime->tm_sec >> 1);
			break;

		case 3:
			//return packed calendar date - (years since 1980)
			//YYYY|YYYM|MMMD|DDDD
			fmtTime = ((newtime->tm_year - 80) << 9) |
						 ((newtime->tm_mon + 1) << 5) |
						 (newtime->tm_mday);
			break;

		default:
			fmtTime = 0;
			break;
	}

	return fmtTime;
}

void
KGetTime(argList)
{
	// return low word of system ticks OR packed real time

	if (argCount == 1)
		sv.acc = (Acc) SysTime(arg(1));
	else
		sv.acc = (Acc) RTickCount();
}

void
KWait(argList)
{
	//	wait the specified number of ticks before returning

	static ulong	lastTick;

	ulong ticks = arg(1);

	if (ticks)
		while (ticks + lastTick > sysTicks)
			;

	sv.acc = (Acc) (sysTicks - lastTick);
	lastTick = sysTicks;

#if 0
	/* This test prevents beta test versions
		from being pirated and other users from using
		this interpreter with any other game then the
		one it was built for
	*/
	if (stampMismatch)
		if (!--stampMismatch)
			PError(E_VER_STAMP_MISMATCH);
#endif
}

