#ifndef EMS_H
#define EMS_H

#if !defined(ALTRES_H)
#include "altres.h"
#endif

extern ARMType	ems;

int			EMSDetect(void);
int			EMSAlloc(word nPages);
int			EMSMapPage(char physicalPage, word logicalPage, word handle);
void			EMSFree(word aHandle);
void far*	EMSGetPageFrame(void);

#endif
