//	armasm.h		Mark Wilden, May 1992
//		interface to assembly helper routines for alternate resource memory access

#if !defined(ARMASM_H)
#define ARMASM_H

#if !defined(ALTRES_H)
#include "altres.h"
#endif

// direction selectors
#define PAGE2MEM	0
#define MEM2PAGE	1

ARMHandle	ARMAlloc(ARMType* arm, unsigned size);
int			ARMCopy(int dir, ARMType* arm, ARMHandle handle, unsigned offset,
					unsigned size, ubyte far* data);
long			ARMMemAvail(ARMType* arm);
void			ARMFree(ARMType* arm, ARMHandle handle);
bool			ARMCritical(void);

#define 		ARMRead(arm, handle, offset, size, data) \
					ARMCopy(PAGE2MEM, arm, handle, offset, size, data)
#define		ARMWrite(arm, handle, offset, size, data) \
					ARMCopy(MEM2PAGE, arm, handle, offset, size, data)

void far*	Get32KEMS(void);
void 			Free32KEMS(void);

#endif
