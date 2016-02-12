/*  Scale.h  */

#include "vm_defs.h"



extern unsigned short vm_deltaxs[ MAXWIDTH ], vm_deltays[ MAXHEIGHT ];



#ifdef	LINT_ARGS

	extern	void		vm_size(word, word, word, word, word, word, 
                             word *, word *, word *, word *);
	extern	void 		GetNewScaleFactors(Obj *, int, word *, word *);


#else

	extern	void		vm_size();
	extern 	void 		GetNewScaleFactors();


#endif


