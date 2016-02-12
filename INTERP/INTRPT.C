//	intrpt.c

#include "errmsg.h"
#include "intrpt.h"

void
InstallServer(fptr proc, word ticks)
{
	// install a server at the head of the timer interrupt chain.  'proc'
	// is a pointer to the server procedure, and 'ticks' is the number of 
	// ticks between service requests.
	
	Server*	cur;
	Server*	freeSlot = NULL;
	
	for (cur = servers; cur < servers + MaxServers; cur++)
		// check to see if this server is already in the list.  If so, return
		if (cur->inUse) {
			if (cur->function == proc)
				return;

		//	otherwise, save this slot
		} else if (!freeSlot)
				freeSlot = cur;
	
	if (!freeSlot)
		Panic(E_MAX_SERVE);

	freeSlot->freq		= ticks;
	freeSlot->count	= ticks;
	freeSlot->function= proc;
	freeSlot->inUse	= TRUE;
}

void
DisposeServer(fptr proc)
{
	// remove the timer interrupt server pointed to by 'proc' from 
	//	the server chain.

	Server*	cur;
	
	_cli();

	for (cur = servers; cur < servers + MaxServers; cur++)
		if (cur->inUse && cur->function == proc) {
			cur->inUse = FALSE;
			break;
		}

	_sti();
}

ulong
RTickCount(void)
{
	return sysTicks;
}
