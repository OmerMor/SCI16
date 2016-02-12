//	mouse.h

#if !defined(MOUSE_H)

#if !defined(GRTYPES_H)
#include	"grtypes.h"
#endif

extern word 	haveMouse;
extern word 	mouseX;
extern word 	mouseY;
extern bool		mouseIsDebug;
extern RRect	restrict;
extern RRect*	restrictRecPtr;
extern word		buttonState;

word	CurMouse(RPoint *);
void	DisposeMouse(void);
void	InstallMouse(void);
word	RGetMouse(RPoint *);
void 	SetMouse(RPoint* pt);

#endif

