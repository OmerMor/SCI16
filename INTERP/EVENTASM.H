#ifndef	EVENTASM_H
#define	EVENTASM_H

extern void		PollKeyboard(void);
extern void		FlushKeyboard(void);
extern int		KeyboardDriver(int, ...);
extern uword	GetModifiers(void);
extern void		PollJoystick(void);
extern int		JoystickDriver(int, ...);

#endif
