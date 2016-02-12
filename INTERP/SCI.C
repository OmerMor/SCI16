//	SCI
// Startup routine for the pseudo machine.

#include "altres.h"
#include "audio.h"
#include "config.h"
#include "ctype.h"
#include "debug.h"
#include "debugasm.h"
#include "dialog.h"
#include "dongle.h"
#include "errmsg.h"
#include "event.h"
#include "fileio.h"
#include "fileload.h"
#include "grtypes.h"
#include "info.h"
#include "intrpt.h"
#include "menu.h"
#include "mouse.h"
#include "palette.h"
#include "picture.h"
#include "pk.h"
#include "resource.h"
#include "savegame.h"
#include "sci.h"
#include "sound.h"
#include "start.h"
#include "stdio.h"
#include "string.h"
#include "window.h"

jmp_buf		restartBuf;
char			whereDisk = '\0';
char			where[64 + 1] = {'\0'};

static void	ReadCommandLineArgs(int	argc, char* argv[]);

void
main(
	int	argc,
	char*	argv[])
{
	LoadLink far** scan;

	ReadCommandLineArgs(argc, argv);

	CheckDongle();

	InitMem(hunkAvail);

	SetInterrupts();
	onexit(ResetInterrupts);

	InitResource(where);

	/* Allocate buffer for use by PK compression routines */
	dcompBuffer = GetResHandle(12574);	// 12574 minimum work buffer for explode

	DebugInit();

	InitErrMsgs();

	/* Wake up the system managers.
	 */
	if (!CInitGraph(videoDriver))
		Panic(E_NO_VIDEO);
	onexit(CEndGraph);

	InitEvent(16);
	onexit(EndEvent);

	InitPalette();

	InstallMouse();

	InitWindow();
	onexit(EndWindow);

	InitDialog(DoAlert);

	InitAudioDriver();
	onexit(EndAudio);

	InitSoundDriver();
	onexit(TermSndDrv);

	if (useAltResMem) {
		AltResMemInit();
		onexit(AltResMemTerm);
	}

	/* Adjust hunkAvail to reflect hunk after drivers are loaded */
	hunkAvail = FreeHunk();

	/* Add any unlocked resources that have been allocated */
	for	(
			scan = (LoadLink far * *) Native(FFirstNode(&loadList));

			scan;

			scan = (LoadLink far * *) Native(FNextNode(Pseudo(scan)))
			)
		{
		if (((*scan)->lock != LOCKED) && !((*scan)->altResMem))
			hunkAvail += (((*scan)->size+15)/16) + 2;
		}

	if (hunkAvail < minHunkSize)
		Panic(E_NO_MEMORY, 16L * (long)(minHunkSize - hunkAvail));

	// Load offsets to often used object properties.  (In script.c)
	LoadPropOffsets();

	// Open up a menu port.
	ROpenPort(&menuPortStruc);
	menuPort = &menuPortStruc;
	InitMenu();

	// Open up a picture window.
	RSetFont(0);
	picWind = RNewWindow(&picRect,NULL, "", NOBORDER | NOSAVE, 0, 1);
	RSetPort((RGrafPort *) picWind);
	InitPicture();

	SaveHeap();

	// We return here on a restart.
	setjmp(restartBuf);

	// Turn control over to the pseudo-machine.
	PMachine();
}

#ifdef DEBUG
char helpstr[] = "\
-a Don't use ARM\n\
-c Debug cursor (one optional parameter)\n\
-m middle mouse button brings up debug\n\
-u resource use info (one optional parameter)\n\
-U hunk use info (one optional parameter)\n\
-w set window size (four required parameters)\n\
-X hunk checksumming\n\
\n\
version %s\n";
#endif

static void
ReadCommandLineArgs(
	int	argc,
	char*	argv[])
{
	/*	we read the command line arguments here, instead of in main, so we
		can keep all argument processing in one place.  -v needs to be
		processed before main(), since it affects how InitMem works (i.e.
		numHandles can change as a result of its inclusion).
		all variables initialized here must be defined with initializers 
		(e.g. int i = 0;) to leave them out of BSS, which will be cleared
		later
	*/
	int	i, j;
	char	str[300];

	*where = 0;
	getcwd(saveDir);
	
	for (i = 1 ; i < argc ; ++i) {
		if (*argv[i] != '-') {
			strncpy(where, argv[i], sizeof(where));
			if (where[1] == ':')
				whereDisk = where[0];
			for (j = 0; where[j] != '\0'; j++)
				saveDir[j] = where[j];
			for (;j >= 0; j--)
				if ((where[j] == ':') ||
					 (where[j] == '\\'))
						break;
			saveDir[++j] = '\0';
         if (saveDir[0] == '\0') getcwd(saveDir);
		} else {
			++(argv[i]); /* skip dash */
			while(*argv[i]) {
				switch (*argv[i]) {
					case 'a':
						useAltResMem = FALSE;
						break;
					case 'c':
						++argv[i];
						if (!isdigit(*argv[i]))
							resourceCursorType = RESCSR_DISKCHANGE	| RESCSR_ARMCHANGE;
						else
							for (resourceCursorType = 0; isdigit(*argv[i]);++argv[i]) {
								resourceCursorType *= 10;
								resourceCursorType += *argv[i] - '0';
							}
						--argv[i];
						break;

					case 'm':
						mouseIsDebug = TRUE;
						break;
					case 'u':
					case 'U':
						argv[i] = ArgNameRead(argv[i]);
						break;
					case 'w':	/* window size: next 4 args are top/left/bottom/right */
						for (i++, picRect.top = 0; isdigit(*argv[i]); ++argv[i])
							picRect.top = picRect.top * 10 + *argv[i] - '0';
						for (i++, picRect.left = 0; isdigit(*argv[i]); ++argv[i])
							picRect.left = picRect.left * 10 + *argv[i] - '0';
						for (i++, picRect.bottom = 0; isdigit(*argv[i]); ++argv[i])
							picRect.bottom = picRect.bottom * 10 + *argv[i] - '0';
						for (i++, picRect.right = 0; isdigit(*argv[i]); ++argv[i])
							picRect.right = picRect.right * 10 + *argv[i] - '0';
						--argv[i];
						break;
					case 'X':
                  /* This option causes a duplicate handle table to be allocated
                     and all handles are in both tables. This is to prevent SC
                     code from stepping on handles. Also a checksum is placed in
                     the loadlinks and the number of loadlinks is tracked. This
                     prevents loadlink from getting over written. */
						checkingLoadLinks = TRUE;
                  /* Reduce the number of handles so we don't run out of Heap space */
                  numHandles = numHandles/2;
						break;
					default:
#ifdef DEBUG
						sprintf(str, helpstr, version);
#else
						sprintf(str, "version %s", version);
#endif
						DoPanic(str);
						break;
			   }
				++(argv[i]);
		   }
	   }
	}
}
