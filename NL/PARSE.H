#ifndef PARSE_H

#define PARSE_H


#ifndef	TYPES_H
#include "types.h"
#endif

#ifndef	LISTS_H
#include "lists.h"
#endif


/*
#define DEBUG
*/

#define MINSOLVESTACK   700
#define MAXWORDS         10

extern bool 	NLDEBUG;
extern bool 	PoolDebug;
extern bool		NL_cantSay;
extern ID	 	nl_event;		/* event passed to parse for "claiming" */
extern uint		MinStackLeft; 
extern int		MAX_NODES_USED;
extern bool		PARSING_USER;
extern nodeptr NL_USER_PARSE_TREE;
extern nodeptr PARSE_NODEPOOL;


#ifdef LINT_ARGS

	void		ParseInit(void);
	bool 		Parse(ID lineID, ID evtID, ID game);

#else

	void		ParseInit();
	bool 		Parse();

#endif


#endif

