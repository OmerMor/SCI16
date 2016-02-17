/* VOCAB.H
 * Definitions for the vocabulary compiler and parser.
 */


#ifndef	VOCAB_H

#define	VOCAB_H

#ifndef	TYPES_H
#include "types.h"
#endif


#define LINESIZE        50
#define MAXPARSENODES   200
#define MAXSAIDNODES    300


/* Structure of dictionary word entries.
 */
typedef struct	AWord
	{
	word	val;	/* word token value 		*/
	word	tags;	/* possible parts of speech */
	word	sibs;	/* index in this array of next sibling */
	word	kids;	/* index in this array of its first kid */
	}
	AWord;

/* Structure of the synonym table for a script.
 */
typedef	struct	synStrc {
	uint		synonym;
	uint		baseWord;
	}
	Synonym;
	
#define	SYN_END		0xffff

/* Structure of a word transformation rule
*/
typedef struct deriv
   {
   strptr derivedStr;
   uint   derivedTags;
   strptr rootStr;
   uint   rootTags;
   }
   Deriv;
   

extern	ID		vocab, vocab_transforms;
extern	uint	wordVal, wordTags;


extern	ID		ParsePoolID;
extern	ID		ParseGrammarID;
extern	ID		SaidPoolID;



#ifdef LINT_ARGS

	void	_far _loadds	InitVocab(void);
	bool			FindWordRoot(strptr);

#else

	void	_far _loadds	InitVocab();
	bool			FindWordRoot();

#endif

#endif

