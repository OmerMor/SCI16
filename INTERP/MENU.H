#if !defined(MENU_H)
#define	MENU_H

#if !defined(KERNEL_H)
#include	"kernel.h"
#endif
#if !defined(PMACHINE_H)
#include	"pmachine.h"
#endif
#if !defined(TYPES_H)
#include	"types.h"
#endif
#if !defined(GRTYPES_H)
#include	"grtypes.h"
#endif

#define	MARKWIDE		8
#define	BARSIZE		10
#define	MAXMENUS		10		// maximum titles in menu bar
#define	CHECKMARK	0x10	// select menu character
#define	FIRST			1		// start index for pointer arrays

typedef struct {
	RRect		bar;
	strptr	text;
	word		value;
	word		state;
	word		key;
	memptr	said;
} RMenuItem;

typedef struct	{
	RRect			bar;
	strptr		text;
	RRect			pageRect;
	word			items;
	Handle		ubits;
	RMenuItem*	item[1];
} MenuPage;

typedef struct {
	RRect			bar;
	word			hidden;
	word			pages;
	MenuPage*	page[1];	
} MenuBar;

extern MenuBar*	theMenuBar;
extern RGrafPort	menuPortStruc;
extern RGrafPort*	menuPort;

void	InitMenu(void);
void	RDrawMenuBar(bool);
word	RMenuSelect(Obj *, word);
void	DrawStatus(strptr, word, word);
void	KDrawStatus(kArgs args);
void	KDrawMenuBar(kArgs args);
void	KMenuSelect(kArgs args);
void	KAddMenu(word *);
void	KSetMenu(word *);
void	KGetMenu(word *);

#endif


