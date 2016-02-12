#ifndef TRIG_H
#define TRIG_H

#include "types.h"

#ifdef LINT_ARGS

	extern   long	ScaledSin(word);
	extern   long	ScaledCos(word);
	extern   int	ATan(int,int,int,int);

#else

	extern   long	ScaledSin();
	extern   long	ScaledCos();
	extern   int	ATan();

#endif


/*	n will be under 1000
	n*TrigScale must be less than largest long int (2,000,000,000)
	so TrigScale<2,000,000
*/

#define TrigScale			(long)10000
#define TrigStep			5

#define SinMult(a,n)	(word)((ScaledSin(a) * n) / TrigScale)		
#define CosMult(a,n)	(word)((ScaledCos(a) * n) / TrigScale)		
#define SinDiv(a,n)	(word)((TrigScale * n) / ScaledSin(a))
#define CosDiv(a,n)	(word)((TrigScale * n) / ScaledCos(a))

#endif /* ifndef TRIG_H */
