/* Project: SCI Interpreter
 *
 * Module:	nl.c
 *
 * Author:	Jeff Stephenson
 *
 * Import, export, and kernel tables for the nl library.
 */



#ifndef	DLL_H
#include	"dll.h"
#endif

extern	void	_far _loadds NLInit(void);
extern	void	_far _loadds Said(void);
extern	void	_far _loadds KSaid(void);
extern	void	_far _loadds KParse(void);
extern	void	_far _loadds KSetSynonyms(void);

LinkTbl	ift = {
	0, "BlkNew", 0,
	0, "Alert", 0, 
	0, "StrPtrGet", 0,
	0, "ResLoad", 0,
	0, "ResLock", 0,
	0, "ResData", 0,
	0, "StrPtrDup", 0,
	0, "InvokeMethod", 0,
	0, "BlkFree", 0,
	0, "SetProperty", 0,
	0, "GetProperty", 0,
	0, "strncpy", 0,
	0, "sprintf", 0,
	0, "strbcat", 0,
	0, "strtrn", 0,
	0, "itoa", 0,
	0, "ModuleFind", 0,
	0, "SeqNew", 0,
	0, "SeqNext", 0,
	0, 0, 0
	};

LinkTbl	eft = {
	NLInit, "NLInit", 0,
	Said, "Said", 0,
	0, 0, 0
	};

KernTbl	kft = {
	KSaid, "Said",
	KParse, "Parse",
	KSetSynonyms, "SetSynonyms",
	0, 0
	};





void	_far _loadds
NLInit()
/* Initialize the Natural Language system.
 */
{
	VocabInit();
	ParseInit();
	SaidInit();
}

