#include "config.h"

#include "resname.h"
#include "types.h"
#include "restypes.h"
#include "string.h"
#include "language.h"
#include "start.h"
#include "fileio.h"
#include "ctype.h"
#include "stdlib.h"
#include "stdio.h"
#include "memmgr.h"
#include "scifgets.h"
#include "dialog.h"
#include	"errmsg.h"
#include "mono.h"

global strptr	videoDriver;
global strptr	soundDriver;
global strptr	kbdDriver;
global strptr	joyDriver;
global strptr	audioDriver;
global strptr	movieDir;
global strptr	patchDir[MAXMASKS];
global bool		useMouse = 1;
global int		useAltResMem = 1;
global uint		audBuffKSize = 4;
global int		audioIRQ = 0;
global int		audioDMA = 0;

global uint		minHunkSize = 0x0000;

global uint		maxHunkUsed = 0;

static char		curDir[] = {".\\"};

static strptr	near gettok(strptr line, strptr tok, strptr sepStr, int size);
static strptr*	near findConfigEntryType(strptr token);
static strptr	near storeStr(strptr str);
static void		near initResMasks(void);

global bool ReadConfigFile(name, defaultName)
strptr	name;
strptr	defaultName;
	/* Read the 'where' file (if it exists) and parse it in order to set up
	 	the paths for drivers and resource files
	*/
{
	static strptr	sepStr = " \t=;,";
	int				fd;
	char				tok[65];
	char				line[100 + 1];
	strptr			str;
	strptr			*pp;

	/*	open either the name passed or the default name	*/
	if (!*name)
		strcpy(name, defaultName);
	if ((fd = open(name, 0)) == -1)
		return 0;

	while (sci_fgets(line, sizeof(line), fd)) {
		/* Get a pointer to the array in which to store this path.
		 * If the input line is not valid, continue to the next line.
		 */
		 
		strlwr(line);
		str = gettok(line, tok, sepStr, sizeof(tok));

		/* See if this is a special case of a mouse driver entry.
		 */
		if (strcmp(tok, "mousedrv") == 0) {
			str = gettok(str, tok, sepStr, sizeof(tok));
			useMouse = tok[0] != 'n' && tok[0] != 'N';
			continue;

		}
		/* ...or whether it's turning off ARM  */
		else if (strcmp(tok, "memorydrv") == 0) {
			str = gettok(str, tok, sepStr, sizeof(tok));
			if (tok[0] == 'n' || tok[0] == 'N')
				useAltResMem = 0;
			continue;
		
		}
		/* ...or whether it's delaring the audio buffer size  */
		else if (strcmp(tok, "audiosize") == 0) {
			str = gettok(str, tok, sepStr, sizeof(tok));
			audBuffKSize = atoi(tok);
			if (audBuffKSize > 64)
				Panic(E_AUDIOSIZE, name, tok);
			continue;
		
		}
		/* ...or whether it's delaring the audio IRQ channel  */
		else if (strcmp(tok, "audioirq") == 0) {
			str = gettok(str, tok, sepStr, sizeof(tok));
			audioIRQ = atoi(tok);
			continue;
		
		}
		/* ...or whether it's delaring the audio DMA channel  */
		else if (strcmp(tok, "audiodma") == 0) {
			str = gettok(str, tok, sepStr, sizeof(tok));
			audioDMA = atoi(tok);
			continue;
		
		}
		
#ifdef SCI
		else if (strcmp(tok, "minhunk") == 0) 
			{
			strptr kloc;
			
			str = gettok(str, tok, sepStr, sizeof(tok));
			kloc = strchr(tok,'k');
			if (!kloc)
				{
				Panic(E_BAD_MINHUNK, name);
				}
			else
				{
				*kloc='\0'; /* isolate the number */
				/* there are 64 paragraphs in a kbyte (1024/16=64) */
				minHunkSize = atoi(tok) * 64;
				}
			continue;
		}
		else if (strcmp(tok, "language") == 0) { /*PG*/
			str = gettok(str, tok, sepStr, sizeof(tok));
			language = atoi(tok);
		}
#endif

		if ((pp = findConfigEntryType(tok)) == NULL)
			/* Either no token on line, or unrecognized token.
			 */
			continue;

		/* Get the various paths for the type of object and store them
		 * in the path array.
		 */
		while (str = gettok(str, tok, sepStr, sizeof(tok)))
			*pp++ = storeStr(tok);
	}
	
	initResMasks();
	close(fd);

#ifdef SCI
	if (minHunkSize==0) 
		{
		Panic(E_NO_MINHUNK, name);
		}
#endif
	
	/* If a patchDir line is not specified, default to
	 * the current directory.
	 */
	if (!patchDir[0]) {
		patchDir[0] = curDir;
	}

	return 1;
}

/**************************************************************************/


static strptr near gettok(str, buf, seps, n)
strptr	str, buf, seps;
int		n;
/* Copy from 'str' the next token (delimited by characters in 'seps') to
 * the storage pointed to by 'buf'.  Return a pointer to the terminating
 * character in 'str', or NULL if no token was copied.  The buffer is
 * 'n' characters in length, so only copy n-1 chars.
 */
 /* formerly in string.c, but resname.c is only client */
{
	/* Start by scanning off any leading seperators.
	 */
	while (*str && strchr(seps, *str))
		++str;
	if (*str == '\0') {
		*buf = '\0';
		return (NULL);
		}
		
	/* Copy a token to storage.
	 */
	while (--n && *str && strchr(seps, *str) == NULL)
		*buf++ = *str++;
	*buf = '\0';
		
	return (str);
}

static strptr * near findConfigEntryType(token)
strptr	token;
{
	int	resType;

	if (resType = ResNameMatch(token)) {
		if (resType == RES_XLATE)
			DoXlate();
		return resTypes[resType - RES_BASE].masks;
	}
	else if (!strcmp(token, "videodrv"))
		return &videoDriver;
	else if (!strcmp(token, "kbddrv"))
		return &kbdDriver;
	else if (!strcmp(token, "joydrv"))
		return &joyDriver;
	else if (!strcmp(token, "sounddrv"))
		return &soundDriver;
	else if (!strcmp(token, "audiodrv"))
		return &audioDriver;
	else if (!strcmp(token, "moviedir"))
		return &movieDir;
	else if (!strcmp(token, "patchdir"))
		return &patchDir[0];
	else 
		return NULL;
}

static strptr near storeStr(str)
strptr	str;
	/* Stores the string 'str' in the static array 'stringStorage' and returns
	 	a pointer to it.
	 	if the path has no extension component (i.e., no '*') allocate enough
	 	space for \*.xxx to be added to it later
	*/
{
	static char	stringStorage[1000];
	static int	stringIndex = 0;
	strptr		ptr;
	int			size = strlen(str) + 1;

	if (!strchr(str, '*'))
		size += 6;

	ptr = &stringStorage[stringIndex];
	stringIndex += size;
	if (stringIndex >= sizeof(stringStorage))
		Panic(E_CONFIG_STORAGE);

	strcpy(ptr, str);

	return (ptr);
}

static void near initResMasks(void)
{
	int		nMasks;
	ResType*	rp;
	char**	mp;
	char*		cp;

	for (rp = resTypes; rp->name; rp++) 
      {
		for (mp = rp->masks, nMasks = 0; *mp; mp++, nMasks++) 
         {
			/*	if no extension component, cat the default on	*/
			cp = strchr(*mp, '*');
			if (!cp) 
            {
				addSlash(*mp);
				strcat(*mp, rp->defaultMask);
			   }
		   }
		
		/*	if no masks, add the default	*/
		if (!nMasks)
			rp->masks[0] = rp->defaultMask;
		/*	use the first one found as the default	*/
		rp->defaultMask = strchr(rp->masks[0], '*');
	   }
}
