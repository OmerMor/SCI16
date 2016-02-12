/*	resname.c	functions using resource file names
					this file is used by several different programs
					Mark Wilden, January 1991
*/

#include "resname.h"
#include "restypes.h"

#include "types.h"
#include "string.h"
#include "io.h"
#include "stdio.h"
#include "stdlib.h"
#include	"errmsg.h"

static strptr	near makeName(strptr dest,strptr mask,strptr name,uint resId);

global ResType	resTypes[] = {
	{ "view", 	"*.v56" },
	{ "pic", 	"*.p56" },
	{ "script",	"*.scr" },
	{ "text",	"*.tex" },
	{ "sound",	"*.snd" },
	{ "memory"			  },
	{ "vocab",	"*.voc" },
	{ "font",	"*.fon" },
	{ "cursor",	"*.cur" },
	{ "patch",	"*.pat" },
	{ "bitmap",	"*.bit" },
	{ "palette","*.pal" },
	{ "wave",	"*.wav" },
	{ "audio",	"*.aud" },
	{ "sync",	"*.syn" },
	{ "message","*.msg" },
	{ "map",		"*.map" },
	{ "heap",	"*.hep" },
	{ "audio36","*.aud" },
	{ "sync36",	"*.syn" },
	{ "xlate",	"*.trn" },
	NULL,
};

static char	near resNameBuf[80];

global char * ResNameMake(resType, resId)
int	resType;
uint	resId;
{
	sprintf(resNameBuf,"%u.%s",resId,resTypes[resType - RES_BASE].defaultMask + 2);
	return resNameBuf;
}

global char * ResNameMakeWildCard(resType)
int	resType;
{
	sprintf(resNameBuf,"*.%s",resTypes[resType - RES_BASE].defaultMask + 2);
	return resNameBuf;
}

global int ResNameMatch(name)
char *	name;
{
	struct ResType*	rp;

	for (rp = resTypes; rp->name; rp++)
		if (!strcmp(name, rp->name))
			return rp - resTypes + RES_BASE;
	return 0;
}

global char * ResName(resType)
int	resType;
{
	return resTypes[resType - RES_BASE].name;
}

global int ROpenResFile(resType, resId, name)
int	resType;
uint  resId;
char*	name;
{
	/*	open a resource file using any of the resource's masks.
		if successful, copies the full name to 'name',
		else nulls it
	*/
	char		fullName[100];
	char**	mask;
	int		fd;

	for (mask = resTypes[resType - RES_BASE].masks; *mask; mask++) {
		makeName(fullName, *mask, name, resId);
		if ((fd = open(fullName, 0)) != -1)
			break;
	}

	if (fd != -1)
		strcpy(name, fullName);
	else
		*name = '\0';
	
	return fd;
}


global char* addSlash(dir)
strptr dir;
{
	int	len;
	char	ch;

	if ((len = strlen(dir)) &&
		 (ch = dir[len - 1]) != '\\' &&
		  ch != '/' &&
		  ch != ':')
		strcat(dir, "\\");
	return dir;
}

/********************** LOCAL FUNCTIONS *****************************/

static strptr near makeName(dest, mask, name, resId)
strptr	dest;
strptr	mask;
strptr	name;
uint		resId;
{
	/*	make a file name based on mask, name, resType and resId
		if new resource names used
			if name is given, just use path component of mask to create name
			otherwise, make name from mask, replacing * with resId
		else
			if name is given, use it, else use ResNameMake
			then prepend mask (which will just be a path) to it
	*/
	char	*cp;
	int	dotIx;

	strcpy(dest, mask);
	if (!(cp = strchr(dest, '*')))
		Panic(E_CONFIG_ERROR, '*', mask);
	dotIx = cp - dest + 1;
	if (name && *name)
		strcpy(cp, name);
	else 
      {
		sprintf(cp, "%u", resId);
		if (mask[dotIx] != '.') 
			Panic(E_CONFIG_ERROR, '.', mask);
		strcat(dest, mask + dotIx);
	   }
	return dest;
}
