/*
** MENU - An attempt at drop down menus on the PC.  A machine with
** system menu support may hook in here
*/

#include	"menu.h"

#include	"start.h"
#include	"grtypes.h"
#include	"graph.h"
#include	"pmachine.h"
#include	"kernel.h"
#include	"event.h"
#include	"string.h"
#include	"stdlib.h"
#include	"ctype.h"
#include	"stdio.h"
#include	"text.h"
#include	"memmgr.h"
#include	"animate.h"
#include	"selector.h"
#include	"sound.h"
#include	"language.h"
#include	"object.h"
#include "mouse.h"
#include	"window.h"
#include	"errmsg.h"

RGrafPort	menuPortStruc;
RGrafPort*	menuPort;

static void	near	ClearBar(word);

//#define MENUBARS

#if	defined(MENUBARS)

static strptr near	GetKeyStr(strptr, word);
static void	near	Invert(word, word);
static word	near	PickUp(word, word);
static word	near	PickDown(word, word);
static void	near	SizePage(MenuPage *);
static word	near	KeySelect(void);
static word	near	FindTitle(RPoint *);
static word	near	FindItem(word, RPoint *);
static void near	DrawMenuBar(word show);
static word	near	MouseSelect(void);
static void	near	DropDown(word);
static void	near	PullUp(word);

/* alt key remapping table */
static	word		altKey[] = {
	30, 48, 46, 32, 18, 33, 34, 35, 23,		/* a - i */
	36, 37, 38, 50, 49, 24, 25, 16, 19,		/* j - r */
	31, 20, 22, 47, 17, 45, 21, 44			/* s - z */
	};

#endif

static strptr statStr = 0;

global void InitMenu()	/* machine specific machine initialization */
{
	theMenuBar = (MenuBar *) NeedPtr(sizeof(MenuBar) + (MAXMENUS * sizeof(MenuPage *)));
	ClearPtr(theMenuBar);
	RSetRect(&theMenuBar->bar, 0, 0, rThePort->portRect.right, BARSIZE);
	theMenuBar->hidden = 1;
	theMenuBar->pages = FIRST;
}



global void KDrawStatus(args) /* build text string from first parameter */
kArgs	args;
{
	RGrafPort	*oldPort;
	word			background = vWHITE;
	word			foreground = vBLACK;
	strptr		str = Native(arg(1));

	if (argCount >= 2) {
		foreground = arg(2);
		if (argCount >= 3)
			background = arg(3);
	}

	RGetPort(&oldPort);
	RSetPort(menuPort);

	if (statStr = str) {
		/* erase the bar rectangle */
		ClearBar(background);
	
		PenColor(foreground);
		RMoveTo(0, 1);

		DrawString(str);

		/* show our handiwork */
		ShowBits(&theMenuBar->bar, VMAP);
	}

	RSetPort(oldPort);
}


static void near ClearBar(color)	/* clear the menu bar to color */
word color;
{
	RGrafPort *oldPort;
	RRect r;

	RGetPort(&oldPort);
	RSetPort(menuPort);

	/* erase the bar rectangle */
	RCopyRect(&theMenuBar->bar, &r);
	RFillRect(&r, VMAP, color);

	/* draw the separator as a black rectangle */
	r.top = r.bottom - 1;
	RFillRect(&r, VMAP, vBLACK);
	RSetPort(oldPort);
}



/* add a menu to the menubar */
global void KAddMenu(args)
argList;
{
#if	!defined(MENUBARS)

	args = args;
	Panic(E_ADDMENU);

#else

	MenuPage *menu;
	RMenuItem *item;
	word m, i, key;
	char valStr[10];
	strptr data;
	word newItem;

	/* if we have no theMenuBar we init one */
	if (!theMenuBar)
		InitMenu();


	/* add to end of existing menubar */
	for (m = FIRST; m <= MAXMENUS; m++){
		if (! theMenuBar->page[m]){

			/* we've got another one */
			++theMenuBar->pages;

			/* determine entries required */
			data = (strptr) Native(arg(2));
			for(i = 1; *data; data++){
				if (*data == ':')
					++i;
			}

			/* allocate a menu for these items */
			theMenuBar->page[m] = menu = (MenuPage *) NeedPtr(sizeof(MenuPage) + (i * sizeof(RMenuItem *)));
			menu->text = Native(arg(1));
			menu->items = FIRST;
		
			/* scan the definition string for item properties */
			i = FIRST;
			data = (strptr) Native(arg(2));
			newItem = 1;
			while (*data){
				if (newItem){
					++menu->items;
					menu->item[i++] = item = (RMenuItem *) NeedPtr(sizeof(RMenuItem));
					ClearPtr(item);
					item->text	= data;
					item->state = dActive;
					newItem = 0;
				}
				switch (*data){
					case ':':
						newItem = 1;
						*data = 0;
						break;
					case '!':
						/* state is inactive */
						item->state = 0;
						*data = 0;
						break;
					case '=':
						/* initial value */
						*data++ = 0;
						key = 0;
						valStr[key] = 0;
						while(isdigit(*data)){
							valStr[key++] = *data++;
							valStr[key] = 0;
						}
						data--;
						item->value = atoi(valStr);								
						break;
					case '`':
						/* next byte(s) are a key code */
						*data++ = 0;
						key = *data++;
						switch(key){
							case '@':
								/* alt key */
								key = toupper(*data);
								item->key = altKey[key - 'A'] << 8;
								break;
							case '#':
								/* function key */
								key = toupper(*data);

								/* allow F10 key as `#0 */
								if (key == '0')
									key += 10;
								item->key = (key - '1' + 0x3b) << 8;
								break;
							case '^':
								key = toupper(*data);
								item->key = key - 64;
								break;
							default:
								--data;
								item->key = toupper(*data);
						
						}
						break;

				}
				++data;
			}

			/* menu is added */
			break;
		}
	}

#endif
}



global void KDrawMenuBar(args)
kArgs	args;
{
#if	!defined(MENUBARS)

	args = args;
	Panic(E_DRAWMENU);

#else

	DrawMenuBar(arg(1));

#endif
}



global void KSetMenu(args)
argList;
{
#if	!defined(MENUBARS)

	args = args;
	Panic(E_SETMENU);

#else

	RMenuItem *item;
	int i;

	item = theMenuBar->page[arg(1) >> 8]->item[arg(1) & 255];
	for (i = 2; i < arg(0); i += 2){
		switch (arg(i)){
			case p_said:
				item->said = (memptr) Native(arg(i + 1));
				break;
			case p_text:
				item->text = Native(arg(i + 1));
				break;
			case p_key:
				item->key = arg(i + 1);
				break;
			case p_state:
				item->state = arg(i + 1);
				break;
			case p_value:
				item->value = arg(i + 1);
				break;
		}
	}

#endif
}

global void KGetMenu(args)
argList;
{
#if	!defined(MENUBARS)

	args = args;
	Panic(E_GETMENU);

#else

	RMenuItem *item;

	item = theMenuBar->page[arg(1) >> 8]->item[arg(1) & 255];
	switch (arg(2)){
		case p_said:
			acc = Pseudo(item->said);
			break;
		case p_text:
			acc = Pseudo(item->text);
			break;
		case p_key:
			acc = item->key;
			break;
		case p_state:
			acc = item->state;
			break;
		case p_value:
			acc = item->value;
			break;
	}

#endif
}


/* give the menu bar a shot at all events */



global void KMenuSelect(args)
kArgs	args;
// Return of -1 indicates no entry selected.  Otherwise, high byte
// is menu number, low byte is number of entry in menu.
{
#if	!defined(MENUBARS)

	args = args;
	Panic(E_MENUSELECT);

#else

	MenuPage 	*menu;
	RMenuItem 	*item;
	memptr		saidSpec;
	word 			type, message, m, i;
	Obj			*event = (Obj *) Native(arg(1));
	int blocks = TRUE;
	
	acc = -1;

	if (argCount == 2 && !arg(2))
		blocks = FALSE;

	/* if we are not inited we return false */
	if (!theMenuBar) {
		acc = 0;
		return;
		}

	/* are we done before we start? */
	if (IndexedProp(event, evClaimed))
		return;

	message = IndexedProp(event, evMsg);
	switch (type = IndexedProp(event, evType)){
		case keyDown:
		case joyDown:
			if (type == joyDown || message == ESC) {
				IndexedProp(event, evClaimed) = TRUE;
				if(blocks) PauseSnd(NULL_OBJ, TRUE);
				acc = KeySelect();
				if(blocks) PauseSnd(NULL_OBJ, FALSE);
				break;
				}

		case saidEvent:
			/* check applicable property against event */
				for (m = FIRST; m < theMenuBar->pages; m++){
					menu = theMenuBar->page[m];
					for (i = FIRST; i < menu->items; i++){
						item = menu->item[i];
						if (item->state & dActive){
							switch (type){
								case keyDown:
									/* make an ASCII message uppercase */
									if (message < 0x100){
										message = toupper((char) message);
									}
									if ((word) item->key && (word) item->key == message){
										IndexedProp(event, evClaimed) = TRUE;
										acc = (i | (m << 8));
									}
									break;
								case saidEvent:
									saidSpec = item->said;
									if (saidSpec){
										IndexedProp(event, evClaimed) = TRUE;
										acc = (i | (m << 8));
									}
									break;
							}
						}
					}
				}
				break;

		case mouseDown:
			if (theMenuBar->bar.bottom > (word) IndexedProp(event, evY)){
				IndexedProp(event, evClaimed) = TRUE;
				if(blocks) PauseSnd(NULL_OBJ, TRUE);
				acc = MouseSelect();
				if(blocks) PauseSnd(NULL_OBJ, FALSE);
			}
			break;
	}

#endif
}


#if	defined(MENUBARS)


static void near	DrawMenuBar(word show)
{
	MenuPage		*menu;
	RGrafPort	*oldPort;
	word			i;
	word			lastX = 8;
	
	RGetPort(&oldPort);
	RSetPort(menuPort);

	if (show){
		theMenuBar->hidden = FALSE;
		ClearBar(vWHITE);
		PenColor(vBLACK);
		/* step through the menu titles and draw them */
		for (i = FIRST; i < theMenuBar->pages; i++){
			menu = theMenuBar->page[i];
			RTextSize(&menu->bar, menu->text, -1, 0);
			menu->bar.bottom =  BARSIZE - 2;
			MoveRect(&menu->bar, lastX, 1);
			RMoveTo(menu->bar.left, menu->bar.top);
			DrawString(menu->text);
			menu->bar.top -= 1;
			lastX = menu->bar.right;
		}
	} else {
		/* hide the bar */
		theMenuBar->hidden = TRUE;
		ClearBar(vBLACK);
	}

	/* show our handy work */
	ShowBits(&theMenuBar->bar, VMAP);

	/* restore old port */
	RSetPort(oldPort);
}


/* make a direction event based selection */ 
static word near KeySelect()	
{
	REventRecord event;
	word done = 0, ret;
	Handle uBits = 0;
	RGrafPort *oldPort;
	word menu, item;		/* here these are indexes */


	RGetPort(&oldPort);
	RSetPort(menuPort);


	/* do we need to redraw */
	if (statStr || theMenuBar->hidden){
		uBits = SaveBits(&theMenuBar->bar, VMAP);
		DrawMenuBar(TRUE);
	}

	/* start with first page */
	DropDown((menu = FIRST));
 	/* first item of page */
	item = PickDown(menu, 0);

	/* flush any events that may have accumulated while drawing */
	RFlushEvents(allEvents);

	while(!done){
		RGetNextEvent(allEvents, &event);
		MapKeyToDir(&event);
		switch (event.type){
			case joyDown:
				done = 1;
				if (item)
					ret =  item | (menu << 8);
				else
					ret = 0;
				break;

			case keyDown:
				switch (event.message){
					case ESC:
						done = 1;
						ret = 0;
						break;
					case CR:
						done = 1;
						if (item)
							ret =  item | (menu << 8);
						else
							ret = 0;
						break;
				}
				break;

			case direction | keyDown:
			case direction:
				switch (event.message){
					case dirE:
						PullUp(menu);
						if ((++menu) >= theMenuBar->pages)
							menu = FIRST;
						DropDown(menu);
						item = PickDown(menu, 0);
						break;
					case dirW:
						PullUp(menu);
						if (!(--menu))
							menu = theMenuBar->pages - 1;
						DropDown(menu);
						item = PickDown(menu, 0);
						break;
					case dirN:
						item = PickUp(menu, item);
						break;
					case dirS:
						item = PickDown(menu, item);
						break;
				}
				/* flush any events that may have accumulated while drawing */
				RFlushEvents(allEvents);
		}
				
	}

	PullUp(menu);

	/* do we need to restore something hidden? */
	if (uBits){
		RestoreBits(uBits);
		ShowBits(&theMenuBar->bar, VMAP);
		theMenuBar->hidden = 1;
	}

	RSetPort(oldPort);
	return(ret);
}




static word	near MouseSelect()	/* return matrix ID of item selected via mouse */
{
	word menu, item;		/* here these are indexes */
	RGrafPort *oldPort;
	RPoint mp;
	Handle uBits = 0;
	
	RGetPort(&oldPort);
	RSetPort(menuPort);

	/* do we need to redraw */
	if (statStr || theMenuBar->hidden){
		/* show our handy work */
		uBits = SaveBits(&theMenuBar->bar, VMAP);
		DrawMenuBar(TRUE);
	}

	/* start with none selected */
	item = menu = 0;
	do {
		RGetMouse(&mp);
		if (RPtInRect(&mp, &theMenuBar->bar)){
			Invert(menu, item);
			item = 0;
			if (menu != FindTitle(&mp)){
				PullUp(menu);
				menu = FindTitle(&mp);
				if (menu)
					DropDown(menu);
			}
		} else {
			if (item != FindItem(menu, &mp)){
				Invert(menu, item);
				item = FindItem(menu, &mp);
				if (item)
					Invert(menu, item);
			}

		}

	} while(RStillDown());

	/* get rid of that mouseUp */
	RFlushEvents(mouseUp);
	
	PullUp(menu);

	/* do we need to restore something hidden? */
	if (uBits){
		RestoreBits(uBits);
		ShowBits(&theMenuBar->bar, VMAP);
		theMenuBar->hidden = 1;
	}

	RSetPort(oldPort);

	/* if item is non zero we have a valid selection */
	if (item)
		item |= (menu << 8);
										   
	return(item);
}


static word near FindTitle(mp)	/* return menu INDEX that mouse may be in */
RPoint *mp;
{
	MenuPage *menu;
	word m;
	RRect r;

	/* step through the menu titles and check for mouse in one of them */
	for (m = FIRST; m < theMenuBar->pages; m++){
		menu = theMenuBar->page[m];
		RCopyRect(&menu->bar, &r);
		++r.bottom;
		if (RPtInRect(mp, &r)){
			return(m);
		}
	}
	return(0);
}


static word near FindItem(m, mp)		/* return item INDEX that mouse may be in */
word m;
RPoint *mp;
{
	RMenuItem *item;
	MenuPage *menu;
	word i;


	/* don't bother with null menu */
	if (m){

		menu = theMenuBar->page[m];
		/* step through the items and check for mouse in one of them */
		for (i = FIRST; i< menu->items; i++){
			item = menu->item[i];
			if ((dActive & item->state) && 	RPtInRect(mp, &item->bar)){
				return(i);
			}
		}
	}
	return(0);
}



static void near Invert(m, i)		/* invert rectangle of this item */
word m, i;
{
	RMenuItem *item;

	if (m && i){
		item = theMenuBar->page[m]->item[i];
		RInvertRect(&item->bar);
		ShowBits(&item->bar, VMAP);
	}
}



static void near DropDown(m)		/* drop down and draw this menu page */
word m;
{
	word i, cnt;
	MenuPage *menu = theMenuBar->page[m];
	RMenuItem *item;

	RRect r;
	word leftX, lastX, lastY = BARSIZE;
	char  text[10];

	/* invert the title */
	RInvertRect(&menu->bar);
	ShowBits(&menu->bar, VMAP);

	/* init the page rectangle to owners size */
	SizePage(menu);

	menu->ubits = SaveBits(&menu->pageRect, VMAP);
	REraseRect(&menu->pageRect);
	RFrameRect(&menu->pageRect);
	ShowBits(&menu->pageRect, (word) VMAP);
	lastX = menu->pageRect.right - 1;
	leftX = menu->pageRect.left + 1;

	/* step through the menu and draw each item */
	for (i = FIRST; i < menu->items; i++){
		item = menu->item[i];

		/* resize left of text rectangle to size of page */
 		item->bar.left = leftX;

		/* resize right of text rectangle to max for all */
 		item->bar.right = lastX;

		/* get set to draw the item */
		RCopyRect(&item->bar, &r);
		REraseRect(&r);
		RMoveTo(r.left, r.top);
		if (dActive & item->state)
			RTextFace(PLAIN);
		else
			RTextFace(DIM);

		/* extend '-' to full width */
		if (*item->text == '-'){
			cnt = (r.right + 1 - r.left) / RCharWidth('-');
			while (cnt--)
				RDrawChar('-');
		} else {
			if (dSelected & item->state){
				RDrawChar(CHECKMARK);
			}
			RMoveTo(r.left, r.top);
			RMove(MARKWIDE, 0);
			ShowString(item->text);
		}

	/* draw in the key equivalent */
		if (item->key){
			GetKeyStr(text, item->key);
			RMoveTo(r.right - RStringWidth(text), r.top);
			ShowString(text);
		}
	}
	RTextFace(0);
	ShowBits(&menu->pageRect, (word) VMAP);
}


static void near PullUp(m)	/* restore bits under this menu */
word m;
{
	MenuPage *menu = theMenuBar->page[m];

	if (m){
		RestoreBits(menu->ubits);
		ReAnimate(&menu->pageRect);
	
		/* re-invert the title */
		RInvertRect(&menu->bar);
		ShowBits(&menu->bar, VMAP);
	}
}



static void near SizePage(menu)	/* step through items to size menu page */
MenuPage *menu;
{
	RMenuItem *item;
	word hasAKey = 0;		/* length of longest key string */
	RRect workRect, r;
	word lastY = BARSIZE;
	ubyte text[10];
	word	i;



	/* init the our rectangle to owners size */
	RCopyRect(&menu->bar, &r);
	++r.bottom;
	r.top = r.bottom;
	r.right = r.left;
	lastY = r.top;

	/* step through the list and size individual rectangles */
	/* Union them into one rectangle */
	for (i = FIRST; i < menu->items; i++){
		item = menu->item[i];
		RTextSize(&workRect, item->text, -1, 0);
		MoveRect(&workRect, r.left, lastY);
		workRect.right += MARKWIDE;

		/* add some room at right if item has a key equivalent */
		if (item->key){
			if (RStringWidth(GetKeyStr(text, item->key)) > hasAKey)
				hasAKey = RStringWidth(GetKeyStr(text, item->key));
		}

		RUnionRect( &workRect, &r, &r);

		/* now give this rectangle to the item */
		RCopyRect(&workRect, &item->bar);

		lastY = r.bottom;

	}

	/* make a little space */
	r.right += 7;
	RInsetRect(&r, -1, -1);

	/* add some space for KEYSTRINGS */ 
	if (hasAKey){
		r.right += hasAKey;
	}


	/* if our rectangle is off the right we slide it back */
	if (r.right >= 320){
		ROffsetRect(&r, 320 - r.right, 0);
	}

	/* set callers rect to this value */
	RCopyRect(&r, &menu->pageRect);
}


static strptr near GetKeyStr(str, key)	/* format a string indicating key equivalence */
strptr	str;
word		key;
{
	int i;

	/* null by default */
	*str = 0;
	if (key < 256){	/* normal alpha or control */
		if (key < ' ') {
			sprintf(str, "%c%c ", 3, key + 0x40);
		} else {
			sprintf(str, "%c ", key);
		}
	} else {
		key /= 256;
		/* function keys */
		if (key >= 59 & key <= 68){
			key -= 58;
			sprintf(str, "F%d ", key);
		} else {

			/* alt keys are selected by occurrence in the alt key table */
			for (i = 0; i < 26; i++){			
				if (altKey[i] == key){
					sprintf(str, "%c%c ", 2, i + 0x41);
					break;
				}
			}
		}
	}
	return(str);
}
	

static word near PickUp(m, i)	/* unhighlight, move up, and re-highlight */
word m, i;
{
	MenuPage *menu = theMenuBar->page[m];
	RMenuItem *item;

	/* we do nothing if we are at top */
	while (i){
		item = menu->item[i];

		/* deselect current */
		if (dActive & item->state){
			RInvertRect(&item->bar);
			ShowBits(&item->bar, VMAP);
		}
		--i;

		if (i){
			item = menu->item[i];
			if (dActive & item->state){
				/* select current and exit */
				RInvertRect(&item->bar);
				ShowBits(&item->bar, VMAP);
				return(i);
			}
		}
	}		
	return(i);
}

static word near PickDown(m, i)	/* unhighlight, move down, and re-highlight */
word m, i;
{
	MenuPage *menu = theMenuBar->page[m];
	RMenuItem *item;
	word newI;

	/* we do nothing if we are at the bottom */
	while ((newI = i + 1) < menu->items){
		if (i){ 
			item = menu->item[i];
			/* deselect current */
			if (dActive & item->state){
				RInvertRect(&item->bar);
				ShowBits(&item->bar, VMAP);
			}
		}
		++i;

		if (i < menu->items){
			item = menu->item[i];
			if (dActive & item->state){
				/* select current and exit */
				RInvertRect(&item->bar);
				ShowBits(&item->bar, VMAP);
				return(i);
			}
		}
	}
	return(i);
}

#endif

