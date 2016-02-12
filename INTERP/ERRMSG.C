/****************************************************************************
 errmsg.c		Chad Bye, Larry Scott, Mark Wilden

 Text error message access

 (c) Sierra On-Line, 1991
*****************************************************************************/

#include	"types.h"
#include	"memmgr.h"
#include	"dialog.h"
#include	"errmsg.h"
#include	"string.h"
#include	"dos.h"
#include	"stdio.h"
#include	"start.h"
#include "text.h"
#include "dialog.h"
#include "stdlib.h"

#define	ERRBUFSIZE	400
#define	FILEBUFSIZE	1000

int	theViewNum;			// for debugging
int	theLoopNum;
int	theCelNum;			// for debugging

static char		errMsgBuf[ERRBUFSIZE];
static char*	errMsgFile = "INTERP.ERR";

static char*	DoReadErrMsg(int errnum, char *textBuf, int msgfile);
static int		OpenErrMsgFile(void);

bool	(*alertProc)(strptr) = (bool (*)(strptr)) DoPanic;

void InitErrMsgs()
{
	char	*tmpBuf;
	int	i, fd;

	if((fd = OpenErrMsgFile()) == -1) {
		sprintf(errMsgBuf, "Can't find %s", errMsgFile);
		DoPanic(errMsgBuf);
	}

	/* Pre-load error messages that cannot be loaded later */
	tmpBuf = errMsgBuf;
	for(i = 1;
		 i <= E_LASTPRELOAD;
		 i++) /* First messages reserved for preloading */
	{
		if(!DoReadErrMsg(i, tmpBuf, fd))
			break;
		while(*tmpBuf++) ;
		lseek(fd, 0L, LSEEK_BEG);		//Messages not necessarily in order
	}
	close(fd);
}

void
SetAlertProc(boolfptr func)
{
	alertProc = func;
}

bool
RAlert(int errnum, ...)
{
	va_list	ap;
	char		msg[ERRBUFSIZE];
	char		buf[ERRBUFSIZE];

	va_start(ap,errnum);
	vsprintf(msg, ReadErrMsg(errnum, buf), ap);
	va_end(ap);

	return alertProc(msg);
}

void
Panic(int errnum, ...)
{
	va_list	ap;
	char		msg[ERRBUFSIZE];
	char		buf[ERRBUFSIZE];

	va_start(ap, errnum);
	vsprintf(msg, ReadErrMsg(errnum, buf), ap);
	va_end(ap);

	sprintf(msg + strlen(msg), ReadErrMsg(E_PANIC_INFO, buf), thisScript, ip);

	DoPanic(msg);
}

global void DoPanic(text)
strptr	text;
{
	panicStr = text;
	exit(1);
}

global bool DoAlert(text)	/* put up alert box and wait for a click */
strptr	text;
{

	word ret = 0;
	RRect ar;
	REventRecord event;
	Handle savedBits;
	RGrafPort *oldPort;

	RGetPort(&oldPort);
	RSetPort(wmgrPort);
	PenColor(0);
	RTextSize(&ar, text, 0, 0);
	if (ar.bottom > 100) {
		RTextSize(&ar, text, 0, 300);
	}
	CenterRect(&ar);
	RInsetRect(&ar, -MARGIN, -MARGIN);
	savedBits = SaveBits(&ar, VMAP);
	RFillRect(&ar, VMAP, 255);
	RFrameRect(&ar);
	ShowBits(&ar, VMAP);
	RInsetRect(&ar, MARGIN, MARGIN);
	RTextBox(text, 1, &ar, TEJUSTLEFT, 0); /*changed from TEJUSTCENTER -PG*/
	while (1){
		RGetNextEvent(allEvents, &event);
		if (event.type == keyDown){
			if (event.message == 27)
				break;
			if (event.message == 0x0d){
				ret = 1;
				break;
			}
		}
	}

	/*	repair the damage */
	RestoreBits(savedBits);
	RInsetRect(&ar, -MARGIN, -MARGIN);
	ShowBits(&ar, VMAP);
	RSetPort(oldPort);
	return (ret);
}

char *ReadErrMsg(int errnum, char *textBuf)
{
	char	*tmpBuf;
	int	j;
	int	fd;

	tmpBuf = errMsgBuf;
	if(errnum <= E_LASTPRELOAD) {
		for(j = 1; j < errnum; j++)
			while(*tmpBuf++) ;
		return(tmpBuf);
	} else {
		if((fd = OpenErrMsgFile()) == -1) {
			sprintf(textBuf, "Can't find %s", errMsgFile);
			return(textBuf);
		}
		if(!DoReadErrMsg(errnum, textBuf, fd))
			sprintf(textBuf, "Error #%d not found in file %s", errnum, errMsgFile);
		close(fd);
		return(textBuf);
	}
}

global void PanicMsgOutput(int errNum)
{
	char str[80];

	Panic(E_VIEW, ReadErrMsg(errNum, str), theViewNum, theLoopNum, theCelNum);
}

static char *DoReadErrMsg(int errnum, char *textBuf, int msgfile)
{
	int	num;
	char	fileBuf[FILEBUFSIZE];
	char	*filePtr = fileBuf;
	char	*linePtr = fileBuf;
	char	*textPtr = textBuf;
	int	count;

	*textPtr = 0;
	count = read(msgfile, fileBuf, FILEBUFSIZE);
	while(count) {
		if(!strncmp(linePtr, "\\\\", 2)) {
			/* found '\\', so read message # */
			linePtr += 2;
			num = 0;
			while((*linePtr >= '0') && (*linePtr <= '9')) {
				num *= 10;
				num += *linePtr++ - '0';
			}

			/* found error message */
			if(num == errnum) {
				/* advance to next line */
				while (count) {
					if(!--count) {
						count = read(msgfile, (filePtr = fileBuf), FILEBUFSIZE);
					}
					if(*filePtr++ == '\n') break;
				}
				linePtr = filePtr;

				while(count) {
					/* if terminating '\\' is found return error message */
					if(!strncmp(linePtr, "\\\\", 2)) {
						/* terminate the string */
						*(textPtr - 1) = 0;
						return(textBuf);
					}

					/* advance to next line while copying to textBuf */
					while(count) {
						if(!--count) {
							count = read(msgfile, (filePtr = fileBuf), FILEBUFSIZE);
						}
						*textPtr++ = *filePtr;
						if(*filePtr++ == '\n') break;
					}
					linePtr = filePtr;
				}
			}
		}

		/* advance to next line */
		while(count) {
			if(!--count) {
				count = read(msgfile, (filePtr = fileBuf), FILEBUFSIZE);
			}
			if(*filePtr++ == '\n') break;
		}
		linePtr = filePtr;
	}
	return(NULL);
}


static int OpenErrMsgFile(void)
{
	int	msgfile;
	char	path[100];

	if((msgfile=open(errMsgFile, 0)) == -1) {
		GetExecDir(path);
		strcat(path, errMsgFile);

		/* read string #errnum from message file and return pointer to it */
		return(open(path, 0));
	}
	return(msgfile);
}
