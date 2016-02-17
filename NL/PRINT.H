#ifndef PRINT_H

#define PRINT_H


#ifndef	PARSE_H
#include "parse.h"
#endif

#ifndef	SYMS_H
#include "syms.h"
#endif

#ifndef	LISTS_H
#include "lists.h"
#endif

#ifndef	QUERY_H
#include	"query.h"
#endif


#define messagesize     500

#define treeprint(strptr, tree, l) nodeprint(strptr, tree, l)


extern char*   message;   /* for Alert to put up debugging messages Q&D */


#ifdef LINT_ARGS

	extern bool nodeAlert(char*,nodeptr);
	extern bool nodeAlert2(char*,nodeptr,nodeptr);
	extern bool queryAlert(char*,NLQUERY);
	extern bool clauseAlert(char*,NLCLAUSE);
	extern bool streamAlert(char*,NLTOK*);

	extern void report_nodepools(void);
	extern void report_nodepool_use(char*,nodeptr,int*);
	extern void ParserProfile(void);

#else

	extern bool nodeAlert();
	extern bool nodeAlert2();
	extern bool queryAlert();
	extern bool clauseAlert();
	extern bool streamAlert();

	extern void report_nodepools();
	extern void report_nodepool_use();
	extern void ParserProfile();

#endif



#endif


