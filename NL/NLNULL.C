/* Project: SCI Interpreter
 *
 * Module:	nlnull.c
 *
 * Author:	Jeff Stephenson
 *
 * A null natural language library for programs which need no parsing.
 */

#ifndef	DLL_H
#include	"dll.h"
#endif


void	_far 	_loadds	NLInit() {};
bool	_far 	_loadds	Said() {return FALSE;};
void	_far 	_loadds	KSaid() {acc = FALSE;};
void	_far 	_loadds	KParse() {acc = FALSE;};
void	_far 	_loadds	KSetSynonyms() {};

LinkTbl	ift = {
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

