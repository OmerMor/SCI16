// SYNC.H

#if !defined(SYNC_H)
#define SYNC_H

#if !defined(TYPES_H)
#include "types.h"
#endif

#if !defined(OBJECT_H)
#include	"object.h"
#endif

/* sync cue structure */
typedef	struct Sync {
	uint	time;		/* absolute frame time to cue */
	uint	cue;		/* cue number to send at frame time */
} Sync;

enum	syncFuncs {
	STARTSYNC,
	NEXTSYNC,
	STOPSYNC,
	QUEUESYNC
};

void	KDoSync(word*);
void	StartSync(Obj* theSync, uint num);
void	StartSync36(Obj*, uint, uchar, uchar, uchar, uchar);
void	QueueSync(Obj* theSync, uint num);
void	QueueSync36(Obj*, uint, uchar, uchar, uchar, uchar);
void	NextSync(Obj* theSync);
void	StopSync(void);
ulong	FindSync36Entry(uint, uchar, uchar, uchar, uchar, uint*);

#endif
