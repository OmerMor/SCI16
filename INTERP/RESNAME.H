#if !defined(RESNAME_H)
#define RESNAME_H

#if !defined(TYPES_H)
#include "types.h"
#endif

#define MAXMASKS		10

typedef struct ResType {
	char	*name;
	char	*defaultMask;
	char	*masks[MAXMASKS];
} ResType;

extern ResType	resTypes[];

char*	ResNameMake(int resType, uint resId);
char*	ResNameMakeWildCard(int resType);
int	ResNameMatch(char * name);
char*	ResName(int resType);
int	ROpenResFile(int resType, uint resId, char* name);
char*	addSlash(strptr dir);

#endif
