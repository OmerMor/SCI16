#ifndef	DIALOG_H

#define	DIALOG_H

#include	"types.h"
#include	"grtypes.h"
#include	"window.h"
#include	"event.h"
#include	"pmachine.h"
#include	"kernel.h"
#include	"intrpt.h"

#define	MARGIN		4

#ifdef	LINT_ARGS

	extern void		InitDialog(boolfptr);
	extern void		RHiliteControl(Obj *s);
	extern word		*DrawControl(Obj *);
	extern word		EditControl(Obj *, Obj *);
	extern int	 	GetLine(strptr, strptr, int);		/* accept or modify the string in buffer */
	extern void		RGlobalToLocal(RPoint *);
	extern void		RLocalToGlobal(RPoint *);
	extern int		EditText(RRect *, strptr, int, int, REventRecord *);
	extern void		DrawCursor(RRect *, strptr, int);
	extern void		EraseCursor(void);

#else


	extern void		InitDialog();
	extern void		RHiliteControl();
	extern word		*DrawControl();
	extern word		EditControl();
	extern int	 	GetLine();		/* accept or modify the string in buffer */
	extern void		RGlobalToLocal();
	extern void		RLocalToGlobal();
	extern int		EditText();
	extern void		DrawCursor();
	extern void		EraseCursor();

#endif

#endif

