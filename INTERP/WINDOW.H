#if !defined(WINDOW_H)
#define WINDOW_H

#if !defined(TYPES_H)
#include "types.h"
#endif

#if !defined(GRTYPES_H)
#include	"grtypes.h"
#endif

#if !defined(LIST_H)
#include	"list.h"
#endif

typedef struct {
	RGrafPort	port;
	RRect			frame;			// visible portion of window
	RRect			saveRect;		// save Area for underBits
	word			wType;			// variation in drawing window
	word			mapSet;			// maps we are in
	Handle		vUnderBits;		// pointer to saved bits in visual map
	Handle		pUnderBits;		// pointer to saved bits in priority map
	strptr		title;			// pointer to name
	word			visible;			// non-zero if window is drawn
} RWindow;

// misc window constants
#define	BRDCOLOR	vBLACK
#define	NOSAVE	1
#define	NOBORDER	2
#define	TITLED	4
#define	STDWIND	0
#define	CUSTOMWIND	0x80

extern RGrafPort*	wmgrPort;
extern List			windowList;
extern uint			winTitleBackColor;
extern uint			winTitleTextColor;

void    	InitWindow(void);			   // init window manager port and window list
void	   EndWindow(void);			   // Release window manager port/window list
RWindow*	RGetWmgrPort(void);
void    	RBeginUpdate(RWindow *);	//	set up for a window update
void    	REndUpdate(RWindow *);		// end update phase of RWindow Manager
void    	RSelectWindow(RWindow *);	// put this window at end of list
RWindow*	RNewWindow(RRect *, RRect *, strptr, word, word, word);
void    	RDrawWindow(RWindow *);		// do actual drawing
void    	RDisposeWindow(RWindow *, word);	// get rid of all allocated storage
void    	RFrameRect(RRect *);			// draw a frame around this rect
void    	CenterRect(RRect *);			// center passed rectangle in wmgrPort
void    	RSetRect(RRect *, word, word, word, word);	// make a rectangle from these values
void    	RCopyRect(RRect *, RRect *);	// copy src is on the left
bool    	REmptyRect(RRect *);			// left >= right or top >= bottom
void    	RUnionRect(RRect *, RRect *, RRect *);	// set ru to a rectangle encompassing both
bool		RFrontWindow(RWindow *);
bool		RPtInRect(RPoint *, RRect *);
int		RPtToAngle(RPoint *, RPoint *);

#endif
