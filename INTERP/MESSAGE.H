// message.h

#if !defined(MESSAGE_H)
#define MESSAGE_H

#if !defined(TYPES_H)
#include	"types.h"
#endif

#define MessageMajorVersion	4000

typedef struct {
	uchar		noun;
	uchar		verb;
	uchar		cond;
	uchar		seq;
	uchar		talker;
	uint		offset;
	uchar		refNoun;
	uchar		refVerb;
	uchar		refCond;
	uchar		refSeq;
} IndexEntry;

typedef struct {
	ulong			version;
	uint			length;			// size of file starting at count field, excluding
										//	comments
	uint			lastMsgNum;
	uint			count;
	IndexEntry	entries[1];
} MsgData;

#endif

