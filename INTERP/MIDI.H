#ifndef MIDI_H
#define MIDI_H

#ifdef	LINT_ARGS

	extern int DoSound(int, ...);
	extern void SoundServer(void);

#else

	extern int DoSound();
	extern void SoundServer();

#endif
#endif
