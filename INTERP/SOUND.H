// SOUND.H

#if !defined(SOUND_H)
#define SOUND_H

#if !defined(OBJECT_H)
#include	"object.h"
#endif

#if !defined(DRIVER_H)
#include	"driver.h"
#endif

#if !defined(LIST_H)
#include	"list.h"
#endif

/*	Sound Node Structure
 */

typedef struct sndStruc {
	int sNext;
	int sPrev;
	int sKey;
	int sNumber;
	char far *sPointer;
	unsigned int tIndex[16];
	unsigned int tLoopPoint[16];
	unsigned int tRest[16];
	unsigned int tLoopRest[16];
	unsigned char tChannel[16];
	unsigned char tCommand[16];
	unsigned char tLoopCommand[16];
	unsigned int cDamprPbend[15];
	unsigned char cPriVoice[15];
	unsigned char cModulation[15];
	unsigned char cPan[15];
	unsigned char cVolume[15];
	unsigned char cProgram[15];
	unsigned char cCurNote[15];  
	unsigned char cFlags[15];  
	unsigned char cMute[15];  
	unsigned int sDataInc;
	unsigned int sTimer;
	unsigned int sLoopTime;
	unsigned char sSignal;
	unsigned char sState;
	unsigned char sHold;
	unsigned char sFixedPri;
	unsigned char sPriority;
	unsigned char sLoop;
	unsigned char sVolume;
	unsigned char sReverbMode;
	unsigned char sFadeDest;
	unsigned char sFadeTicks;
	unsigned char sFadeCount;
	unsigned char sFadeSteps;
	unsigned char sPause;
	unsigned char sSample;
} Sound;


enum	sndFuncs {
	MASTERVOL,
	SOUNDON,
	RESTORESND,
	NUMVOICES,
	NUMDACS,
	SUSPEND,
	INITSOUND,
	KILLSOUND,
	PLAYSOUND,
	STOPSOUND,
	PAUSESOUND,
	FADESOUND,
	HOLDSOUND,
	MUTESOUND,
	SETVOL,
	SETPRI,
	SETLOOP,
	UPDATECUES,
	MIDISEND,
	SETREVERB,

	CHANGESNDSTATE
	};



/*	MIDI.S functions
 */
#define	SPatchReq	0
#define	SInit			1
#define	STerminate	2
#define	SProcess		3
#define	SSoundOn		4
#define	SRestore		5
#define	SMasterVol	6
#define	SSetReverb	7
#define	SPlay			8
#define	SEnd			9
#define	SPause		10
#define	SFade			11
#define	SHold			12
#define	SMute			13
#define	SChangeVol	14
#define	SChangePri	15
#define	SGetSignal	16
#define	SGetDataInc	17
#define	SGetSYMPTE	18
#define	SNoteOff		19
#define	SNoteOn		20
#define	SController	21
#define	SPChange		22
#define	SPBend		23
#define	SAskDriver	24
#define	SGetSignalRset	25


/* MIDI commands
 */
#define	NOTEOFF		0x80
#define	NOTEON		0x90
#define	POLYAFTER	0xa0
#define	CONTROLLER	0xb0
#define	PCHANGE		0xc0
#define	CHNLAFTER	0xd0
#define	PBEND			0xe0


/* defines for flags property
 * of sound objects
 */
#define	mNOPAUSE		0x0001
#define	mFIXEDPRI	0x0002

extern int	reverbDefault;
extern List	soundList;
extern bool	soundOn;

bool InitSoundDriver(void);
void TermSndDrv(void);
void KillAllSounds(void);
void RestoreAllSounds(void);
ubyte GetSoundResType(uint);
void KDoSound(word *);
void SuspendSounds(int);
void InitSnd(Obj *);
void KillSnd(Obj *);
void PlaySnd(Obj *,int);
void StopSnd(Obj *);
void PauseSnd(Obj *,int);
void FadeSnd(Obj *,int,int,int,int);
void HoldSnd(Obj *,int);
void SetSndVol(Obj *,int);
void SetSndPri(Obj *,int);
void SetSndLoop(Obj *,int);
void UpdateCues(Obj *);
void MidiSend(Obj *,int,int,int,int);
void ChangeSndState(Obj *);
void SampleCues(Obj *);
void PlaySamp(Obj *,int);
void InitSamp(Obj *);

#endif

