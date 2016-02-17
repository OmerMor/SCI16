#ifndef SAID_H

#define SAID_H


#ifndef	TYPES_H
#include "types.h"
#endif

#ifndef	SYMS_H
#include "syms.h"
#endif

#ifndef	LISTS_H
#include "lists.h"
#endif

#ifndef	SCI_H
#include	"sci.h"
#endif


#define MaxSaidSize   50

#define HIBIT 0x80


#define hibiton(abyte)        ((abyte) & HIBIT)

#define EOspec(abyte)         (abyte ==  METAEND)


extern bool    SaidDebug;
extern bool    PARSING_SAID;
extern int     MAX_SAIDNODES_USED;
extern NLTOK*  intokens;
extern nodeptr NLSFpool;      /* Allocated node pool from hunk */
extern nodeptr Said_NODEPOOL; 

#ifdef LINT_ARGS

void	_far _loadds	KSaid(kArgs args);
bool	_far _loadds	Said(ID specID, ID eventID);

#else

void	_far _loadds	KSaid();
bool	_far _loadds	Said();

#endif


#endif

