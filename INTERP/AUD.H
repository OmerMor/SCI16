#ifndef AUD_H
#define AUD_H

#ifdef	LINT_ARGS

	extern int		AudioDrv(int function, uint qualifier);
	extern void		AudioServer(void);
	extern int		CDAudioDrv(int function, uint qualifier);

#else

	extern int		AudioDrv();
	extern void		AudioServer();
	extern int		CDAudioDrv();

#endif
#endif
