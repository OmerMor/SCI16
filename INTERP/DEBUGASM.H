//	debugasm.h

#if !defined(DEBUGASM_H)
#define DEBUGASM_H

void	DebugInit(void);
void	DebugOn(void);
void	DebugOff(void);
void  RDebugger(void);
void  KStackUsage(void);
uint	StackUsage(int);
void	ShowFreeList(strptr);
void	ProfileHandler(void);

#endif
