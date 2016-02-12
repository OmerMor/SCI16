//	altres.h		alternate resource memory manager
//					Mark Wilden, January 1991

#if !defined(ALTRES_H)
#define ALTRES_H

#if !defined(RESOURCE_H)
#include "resource.h"
#endif

#define NARMTYPES	5

typedef struct {
	char*		name;
	char		debugPrefix;

	bool		(*init)(void);
	int		(*memcpy)(void);
	void		(*term)(void);

	uword		pageSize;
	bool		active;

	Handle	pageMap;
	int		nPages;
	int		nFreePages;
} ARMType;

extern int	useAltResMem;

//	resource management functions (ALTRES.C)
void			AltResMemInit(void);
ARMHandle	AltResMemAlloc(uword size, ARMType** type);
Handle		AltResMemRead(LoadLink far** loadLink);
bool			AltResMemWrite(LoadLink far** loadLink);
void			AltResMemFree(LoadLink far** loadLink);
char			AltResMemDebugPrefix(LoadLink far** loadLink);
void			AltResMemDebugSummary(strptr where);
void			AltResMemDebugKey(strptr where);
void			AltResMemTerm(void);

#define NO_MEMORY	-1

#endif
