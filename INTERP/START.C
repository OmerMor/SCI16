//	start.c
//		startup and shutdown routines

#include "ctype.h"
#include "start.h"
#include "string.h"

#define MAX_ON_EXIT	20

//	initialize explicitly because SCI won't have done it for us by now
char*	argv[10] = { 0 };
int	argc = 0;

static int		exitIndex;
static fptr		exitProcs[MAX_ON_EXIT];

static void	WriteString(char*	str);

void
MakeCommandLineArgs(
	char _far* cmdTail)
{
	int			nChars = *cmdTail++;
	char*			cp;
	char far*	fcp;
	int			i;

	if (!nChars)
		return;

	//	copy command line to heap
	fcp = cmdTail;
	cp = commandLine;
	for (i = 0; i < nChars; i++)
		*cp++ = *fcp++;
	*cp = 0;

	argv[argc++] = "sci";

	for (cp = commandLine; *cp; ) {
		//	strip leading whitespace
		while (*cp && isspace(*cp))
			cp++;
		if (!*cp)
			break;

		//	set argument
		argv[argc++] = cp;

		//	find argument end
		while (*cp && !isspace(*cp))
			cp++;
		if (!*cp)
			break;

		//	terminate argument
		*cp++ = 0;
	}
}

void
onexit(
	fptr	func)
{
	//	Add a procedure to the list of those to be executed on exit

	if (exitIndex <= MAX_ON_EXIT - 1)
		exitProcs[exitIndex++] = func;
}

void
exit(
	char	code)
{
	int	i;

	for (i = exitIndex - 1; i >= 0; i--)
		exitProcs[i]();

	if (panicStr)
		WriteString(panicStr);
	else if (quitStr)
		WriteString(quitStr);

	ExitFromC(code);
}

static void
WriteString(
	char*	str)
{
	//	write a text string to the screen on exit

	while (*str) {
		if (*str == '\n')
			WriteChar('\r');
		WriteChar(*str);
		++str;
	}
}
