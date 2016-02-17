/*	NLVOCAB    : Dictionary and word-transform manager
	
	Subprogram : NLP (Natural Language Parser Interface)
	
	Program    : SCI (Script Interpreter for Sierra On-line games)
	
	Author/Date: Jeff Stephenson, dictionary
	             Pablo Ghenis, word transforms
	             for Sierra On-line, July 1988
	
	Platform   : Portable C, any 16+ bit micro with 512k+ RAM
	
	Purpose    :



***************************************************************************/



#ifndef	TYPES_H
#include	"types.h"
#endif

#ifndef	DLL_H
#include	"dll.h"
#endif

#ifndef	SEQUENCE_H
#include	"sequence.h"
#endif

#ifndef	RESOURCE_H
#include	"resource.h"
#endif

#ifndef	MODULE_H
#include	"module.h"
#endif

#ifndef	STRING_H
#include	"string.h"
#endif

#ifndef	SELECTOR_H
#include	"selector.h"
#endif

#ifndef	VOCAB_H
#include	"vocab.h"
#endif

#ifndef	LISTS_H
#include "lists.h"
#endif


#define	MAXSYNS		10
#define	TERMBIT		0x80


uint		wordVal = 0, wordTags = 0;

ID		vocab = 0;
ID		vocab_transforms = 0;
ID		synArray[20];


#ifdef	LINT_ARGS

	static	bool		FindWord(strptr);
	static	uint		CheckSynonyms(uint);
	static	memptr	NextWord(memptr);
						
   static	bool 		tryRule(strptr,Deriv*);
   static	memptr 	nextRule(memptr,Deriv*);

#ifdef notdef   
	static	void		showRules(void);
	static	bool		showRule(Deriv*);
#endif

#else

	static	bool		FindWord();
	static	uint		CheckSynonyms();
	static	memptr	NextWord();
						
   static	bool 		tryRule();
   static	memptr 	nextRule();

#ifdef notdef   
	static	void		showRules();
	static	bool		showRule();
#endif

#endif /* #ifdef-else LINT_ARGS */




void
VocabInit()
/* Read in the vocabulary file.
 */
{
	vocab = ResLoad(RES_VOCAB, 0);              /* load dictionary */
	ResLock(RES_VOCAB, 0, TRUE);
	
	vocab_transforms = ResLoad(RES_VOCAB, 901);  /* load word transforms */
	ResLock(RES_VOCAB, 901, TRUE);
}



bool
FindWordRoot(wp)
strptr	wp;
{
   char		ds[LINESIZE], rs[LINESIZE];
   Deriv		Rule, *Ruleptr;
	memptr	RuleDB;
	
	char		theWord[50];

	/* Copy word to our local storage, convert to lower case, reset pointer.
	 */
	strcpy(theWord, wp);
	strlwr(theWord);
	wp = theWord;
 
   if (FindWord(wp)) /* if in vocab then all is OK */
      return TRUE;
    
   if (isdigit(*wp)) /* we don't transform numbers... */
      return FALSE;
   
   /* point to allocated storage */  
   Rule.derivedStr = ds;
   Rule.rootStr    = rs; 
	RuleDB = ResData(vocab_transforms);
   Ruleptr = &Rule;
   
   while ((RuleDB = nextRule(RuleDB, Ruleptr)) != NULL)
      if (tryRule(wp,Ruleptr))
         return TRUE;

   wordVal=wordTags=0;  /* if no rule hit then we don't know */
   return FALSE;
}



bool
FindWord(wp)
strptr	wp;		/* word pointer */
/* Look up the word pointed to by 'wp' in the vocabulary table.  If not found,
 * return FALSE.  Otherwise, put the word value in 'wordVal', the word tags
 * in 'wordTags', and return TRUE.
 */
{
	memptr	vp;	/* vocabulary pointer */
	int		inCommon;
	uint		val;

	/* If word does not start with a letter, point right past the directory.
	 * Otherwise, use the directory to look up the first word starting with
	 * the appropriate letter.
	 */
	vp = ResData(vocab);
	if (!islower(*wp))
		vp += 26 * sizeof(int);
	else {
		vp += 2 * (*wp - 'a');
		if (*vp == 0 && *(vp+1) == 0)
			return (FALSE);
		vp = ResData(vocab) + *vp + *(vp+1) * 0x100;
		}

	/* Start looking for the word.
	 */
	for (inCommon = 0 ; ; vp = NextWord(vp)) {
		/* If the word in the vocabulary has fewer letters in common with
		 * the previous word than 'wp' does, there can be no match.
		 */
		if ((int) *vp < inCommon)
			return (FALSE);

		/* If the word in the vocabulary has more letters in common with
		 * the previous word than 'wp' does, continue looking.
		 */
		if ((int) *vp > inCommon)
			continue;

		/* Scan the word currently pointed to until we get a mismatch
		 * of characters.
		 */
		for (++vp ; *vp == *wp ; ++vp, ++wp)
			++inCommon;

		/* If the characters mismatch only because the TERMBIT of vp is
		 * set, then we have a match if the next char in wp is also a
		 * terminator.
		 */
		if ((*vp & ~TERMBIT) == *wp) {
			++inCommon;
			++wp;
			if (*wp == '\0') {
				++vp;
				wordTags = *vp++ << 4;
				wordTags += *vp >> 4;
				val = (*vp++ & 0x0f) << 8;
				val += *vp;
/*
				val = (ulong)*vp++ << 16;
				val += *vp++ << 8;
				val += *vp;
				wordTags = (uint) (val >> 12);
*/
				wordVal = CheckSynonyms(val);
				return (TRUE);
				}
			}
		}
}




memptr
NextWord(vp)
memptr	vp;
/* Point to the next word in the vocabulary.
 */
{
	/* Scan to the end of the current word.
	 */
	while ((*vp & TERMBIT) == 0)
		++vp;

	/* Point past the tags and value.
	 */
	return (vp + 4);
}






uint
CheckSynonyms(theWord)
uint	theWord;
/* Search the synonyms for a match to this word and return
 * the base word if a match is found.
 */
{
	int		i;
	ID			synID;
	Synonym	*sp;

	for (i = 0 ; i < MAXSYNS && (synID = synArray[i]) ; ++i) {
		BlkPtrGet(synID, sp);
		for ( ; sp->synonym != SYN_END ; ++sp)
			if (sp->synonym == theWord)
				return (sp->baseWord);
		}

	return (theWord);
}



bool
tryRule(wp,Ruleptr)
strptr	wp;
Deriv		*Ruleptr;
{
   char newWord[LINESIZE]; /*used by tryRoot macro*/
   
   if ( strtrn(wp,Ruleptr->derivedStr,
               Ruleptr->rootStr,newWord)      /* does pattern match? */
        && FindWord(newWord)                  /* root in dictionary? */
        && (wordTags & Ruleptr->rootTags)!=0  /* are tags right? */
      )
      {
      wordTags=Ruleptr->derivedTags;   /* then set tags
                                          wordVal was set by FindWord */
      return TRUE;
      }
   else
      return FALSE;
}



#ifdef notdef   

void
showRules()
{
   char ds[LINESIZE], rs[LINESIZE];
	Deriv *Ruleptr,Rule;
   memptr RuleDB;
	
   RuleDB   = *vocab_transforms;

   Rule.derivedStr = ds;
   Rule.rootStr    = rs;
   
   Ruleptr = &Rule;
   
   for (;(RuleDB=nextRule(RuleDB,Ruleptr))!=(memptr)NULL;)
		{
		if (!showRule(Ruleptr))
			break;
		}
	}
	/*end showRules*/

bool showRule(Ruleptr)
	Deriv* Ruleptr;
	{
	char message[80]; 
	
	sprintf(message,"(%s %x %s %x)",
	        Ruleptr->derivedStr,
	        Ruleptr->derivedTags,
	        Ruleptr->rootStr,
	        Ruleptr->rootTags);
	        
	return Alert(message);
}

	
#endif /*ifdef notdef*/



memptr nextRule(RuleDB,Ruleptr)
memptr	RuleDB; /* pointer into rule database resource */
Deriv		*Ruleptr;  /* must be pointing to allocated strings!!! */
{
   strptr s;
   
   if (*RuleDB==0) 
      return (memptr)NULL;
     
   s = Ruleptr->derivedStr;
   while(*s++=*RuleDB++);                       /* skip past terminator */
                                            
   Ruleptr->derivedTags=*RuleDB++<<8;           /* two-byte derived tags */
   Ruleptr->derivedTags+=*RuleDB++;
   
   s = Ruleptr->rootStr;
   while(*s++=*RuleDB++);                       /* skip past terminator */
                                            
   Ruleptr->rootTags=*RuleDB++<<8;              /* two-byte root tags */
   Ruleptr->rootTags+=*RuleDB++; 
   
   return RuleDB;
}



void	_far _loadds
KSetSynonyms(args)
kArgs		args;
/* Put pointers to the synonyms of each Region in the region list in the
 * array 'synArray'.
 */
{
	register	ID		seq;
	register	int		i;
	ID			regListID, regID, moduleID;
	Module	*mp;

	/* Get the actual list of Regions from the Region Collection instance.
	 */
	regListID = GetProperty(arg(1), s_elements);

	/* Get the synonyms for each region in the list.
	 */
	i = 0;
	seq = SeqNew(regListID, SEQ_SHARE);
	while (regID = SeqNext(seq)) {
		/* Find the module block for this region.
		 */
		moduleID = ModuleFind(GetProperty(regID, s_number));

		/* If this module has synonyms, add them to the synonym array.
		 */
		BlkPtrGet(moduleID, mp);
		if (mp && mp->synonyms && i < MAXSYNS)
			synArray[i++] = mp->synonyms;
		}
	SeqFree(seq);

	if (i < MAXSYNS)
		synArray[i] = 0;
}



strptr
strlwr(s)
register	strptr	s;
{
	register	strptr	t;

	for (t = s ; *t != '\0' ; ++t)
		*t = tolower(*t);

	return (s);
}



/*
strptr
strcpy(d, s)
register	strptr	s, d;
{
	strptr	ptr = d;

	while (*d++ = *s++)
		;

	return (ptr);
}
*/

