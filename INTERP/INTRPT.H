// INTRPT.H

#if !defined(INTRPT_H)
#define INTRPT_H

#if !defined(TYPES_H)
#include	"types.h"
#endif

typedef struct {
	word	inUse;		// is server active?
	word	freq;			// desired freqency of service
	word	count;		// count of ticks to next service
	fptr	function;	// pointer to server function
} Server;

#define MaxServers	4

extern Server	servers[];
extern ulong	sysTicks;
extern bool		diskIOCritical;

void		SetInterrupts(void);
void		ResetInterrupts(void);
void		SetDisneyInt(void);
void		InstallServer(fptr, word);
void		DisposeServer(fptr);
void		_cli(void);
void		_sti(void);
ulong		RTickCount(void);
word		SysTime(word);

#endif
