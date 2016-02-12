#if !defined(ANIMATE_H)
#define ANIMATE_H

#if !defined(TYPES_H)
#include	"types.h"
#endif

#if !defined(GRTYPES_H)
#include	"grtypes.h"
#endif

#if !defined(LIST_H)
#include	"list.h"
#endif

// states of actor's "s_signal" property 
#define	UPDBITS		0x0007	// the update bits
#define	STOPUPD		0x0001	// init "stopped condition
#define	STARTUPD		0x0002	// abandon "stopped" condition
#define	NOUPDATE		0x0004	// actor is stopped
#define	HIDE			0x0008	// actor is not to be drawn
#define	FIXPRI		0x0010	// use priority in actor
#define	VIEWADDED	0x0020	// leave view in picture
#define	FORCEUPD		0x0040	// update a stopped actor this time only
#define	HIDDEN		0x0080	// an actor to HIDE has been hidden 

#define	staticView	0x0100	// no doit: call required 
#define	blocked		0x0400	// tried to move, but couldn't 
#define	fixedLoop	0x0800	// loop is fixed 
#define	fixedCel		0x1000	//	cel fixed
#define	ignrHrz		0x2000	// can ignore horizon
#define	ignrAct		0x4000	// can ignore other actors
#define	delObj		0x8000

extern List*	lastCast;

void		DisposeLastCast(void);
void		ReAnimate(RRect*);
void		Animate(List*, word);
void		AddToPic(List*);

#endif


