#ifndef SYMS_H

#define SYMS_H


#ifndef	TYPES_H
#include "types.h"
#endif


#define NLSYMSIZE 30

/* Part of speech tag values.
 */
#define	VERB		0x800
#define	ADV		0x400
#define	AUXV		0x200
#define	NOUN		0x100
#define	PRON		0x080
#define	ADJ		0x040
#define	ART		0x020
#define	POS		0x010
#define	ASS		0x008
#define	CONJ		0x004

/* extras that might be useful
   - Pablo Ghenis 6/16/88 */

#define	PUNCTTAG	0x002
#define	NUMTAG	0x001

/* Meta-string codes.
 */
#define	ORSAID		0xf0
#define	ANDSAID		0xf1
#define	NEXTPART		0xf2
#define	LEFTPAREN	0xf3
#define	RIGHTPAREN	0xf4
#define	OPTBEGIN		0xf5
#define	OPTEND		0xf6
#define	NUMBER		0xf7
#define	MODIFIES		0xf8
#define	RUNON			0xf9
#define	METAEND		0xff

#define METASHIFT 0x100

#define NLRNON   RUNON * METASHIFT 
#define NLLESS   MODIFIES * METASHIFT
#define NLSLSH   NEXTPART * METASHIFT
#define NLLP     LEFTPAREN * METASHIFT
#define NLRP     RIGHTPAREN * METASHIFT
#define NLLB     OPTBEGIN * METASHIFT
#define NLRB     OPTEND * METASHIFT
#define NLCOMA   ORSAID * METASHIFT
#define NLAND    ANDSAID * METASHIFT
#define NLEOSPEC METAEND * METASHIFT
#define NLEND    METAEND * METASHIFT



typedef char*     NLSYM;
typedef uword     NLTOK;
typedef uword     NLTAGS;

typedef
enum NLTOKEN
     {
     MINNLTOKEN=300,
     /* NLRP and NLLP are also defined in nlsdgrmr.h!
     NLRP,
     NLLP,
     */
     NLVERB=303,
     NLNOUN,
     NLART,
     NLADJ,
	  NLPREP,
	  NLPRON,
     NLCONJ,
     NLAUXV,
     NLADV,
     NLASS,
     NLCNOUN,
     NLCADJ,
     NLCVERB,
     NLVP,
     NLNP,
	  NLAP,
	  NLS,
	  NLEOL,
	  NLROOT,
	  NLDOBJ,
	  NLIOBJ,
	  NLMOD,
	  NLIGNR,
	  NLVOCAB,
	  NLSPEC,
	  NLSPEC2,
	  NLPHR,
	  NLPHR2,
	  NLMORE,
	  NLUNIT,
	  NLPNCT,
	  NLEMPT,
	  NLOR,
	  NLOR2,
	  NLATOM,
	  NLOPT,
	  NLWORD,
	  NLPUT,
	  NLNUM,
     EOGRAMMAR,
     MAXNLTOKEN
     }
     NLTOKEN;

extern NLSYM NLSYMTAB[];

#ifdef LINT_ARGS

	extern NLTOK sym_to_tok(NLSYM,NLSYM*);
	extern bool tok_to_sym(NLTOK, NLSYM*, NLSYM);

#else

	extern NLTOK sym_to_tok();
	extern bool tok_to_sym();

#endif

#endif

