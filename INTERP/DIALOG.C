/*
** DIALOG - All user interaction routines.
** Contains control manager functions also
*/


#include	"dialog.h"
#include	"selector.h"
#include "graph.h"
#include	"types.h"
#include	"grtypes.h"
#include	"pmachine.h"
#include	"kernel.h"
#include	"event.h"
#include	"string.h"
#include	"text.h"
#include	"memmgr.h"
#include	"menu.h"
#include	"animate.h"
#include	"resource.h"
#include	"cels.h"
#include "language.h"
#include "start.h"
#include "object.h"
#include "window.h"
#include "stdio.h"
#include "stdarg.h"
#include "start.h"
#include "dos.h"
#include "errmsg.h"

struct TEdit {
	strptr	text;
	RRect	box;
	word	cursor;
	word	max;
};
typedef	struct TEdit TEdit;

static	word	near curOn;
static	long	near flash;
static	RRect	near curRect;

#ifdef	LINT_ARGS

	static void near	FlashCursor(void);
	static void near	ToggleCursor(void);
	static void near	SetFlash(void);
	static void near 	DrawSelector(Obj *);
	static void near	TextEdit(Obj *, Obj *);

#else

	static void near	FlashCursor();
	static void near	ToggleCursor();
	static void near	SetFlash();
	static void near	DrawSelector();
	static void near	TextEdit();

#endif



global void InitDialog(proc)
boolfptr	proc;
{
	if (proc){
		/* load and lock FONT.000 */
		ResLoad(RES_FONT, 0);
		ResLock(RES_FONT, 0, TRUE);
		/* now set the alert procedure */
		SetAlertProc(proc);
	}
}




global void DrawCursor(box, textBuf, cursor)
RRect		*box;
strptr	textBuf;
int		cursor;
{
	strptr	text;

	if (!curOn) {
		curRect.left = box->left + RTextWidth(textBuf, 0, cursor, GetFont());
		curRect.top = box->top;
		curRect.bottom = curRect.top + GetPointSize();

		/* we flash the character cel unless we are at end of string */
		text = textBuf + cursor;
		curRect.right = curRect.left + ((*text)? RCharWidth(*text) : 1);

		ToggleCursor();
		}
	curOn = TRUE;
	SetFlash();
}



global void EraseCursor()
{
	if (curOn)
		ToggleCursor();
	curOn = FALSE;
	SetFlash();
}



static void near	FlashCursor()
{
	if (flash < RTickCount()) {
		ToggleCursor();
		curOn = !curOn;
		SetFlash();
		}
}



static void near	ToggleCursor()
{
	RInvertRect(&curRect);
	ShowBits(&curRect, VMAP);
}



static void near	SetFlash()
{
	flash = 30L + RTickCount();
}



global word EditControl(item, evt)
Obj	*item, *evt;
{
	word oldFont;
	
	/* if this is a NULL control (zero) exit */
	if (!item)
		return(0);

	switch (GetProperty(item, s_type)){
		case dEdit:

			oldFont = GetFont();
			RSetFont(GetProperty(item, s_font));

			/* we are ready to deal with the event */
			if (!evt)
				EraseCursor();
			else {
				switch(GetProperty(evt, s_type)) {
					case keyDown:
						switch (GetProperty(evt, s_message)) {
							case ESC:
							case CR:
								EraseCursor();
								break;
							case '|':
								//	ignore | since it has special meaning to text
								//	output routines
								break;
							default:					
								TextEdit(item, evt);
								break;
							}
						break;
					default:
						TextEdit(item, evt);
						break;
					}
				}

			/* restore old font */
			RSetFont(oldFont);
			break;
	}
	return(0);
}



global void RHiliteControl(item)		/* hilight (or un-highlight) the control */
Obj	*item;
{
	RRect r;

	/* all items need their now seen rectangles sized externally */
	RCopyRect(GetPropAddr(item, s_nowSeen), &r);

	/* leave r set to encompass the entire area */
	switch (GetProperty(item, s_type)) {
		case dText:
		case dButton:
		case dEdit:
		case dIcon:
		case dMenu:
			RInvertRect(&r);
			break;
		default:
			break;
		}

/* show the rectangle we affected */
	ShowBits(&r, VMAP);

}



global word *DrawControl(item)	/* draw this control */
Obj	*item;
{
	RRect		r, ur;
	strptr	text;
	int		font, oldFont, type, state;
   word     *nRect;

   nRect = NULL;

	type = GetProperty(item, s_type);
	if (type == dEdit)
		EraseCursor();
	state = GetProperty(item, s_state);

	/* all items need their now seen rectangles sized externally */
	RCopyRect(GetPropAddr(item, s_nowSeen), &r);


	/* get some properties up front */
	if (RespondsTo(item, s_text)) {
		text = Native(GetProperty(item, s_text));
		}
	if (RespondsTo(item, s_font))
		font = GetProperty(item, s_font);

	/* leave ur set to encompass the entire area affected */
	switch (type) {
		case dSelector:
		case dScroller:
			RInsetRect(&r, -1, -1);
			RCopyRect(&r, &ur);
			DrawSelector(item);
			break;

		case dButton:
			/* ensure that minimum control area is clear */
			RInsetRect(&r, -1, -1);
			REraseRect(&r);
/* 			RFillRect(&r, VMAP, 8);		 Used to be green, now lt black */
			RFrameRect(&r);
			RCopyRect(&r, &ur);

			/* box is sized 1 larger in intrface */
			RInsetRect(&r, 2, 2);
/*			PenColor(0);		*/
			if (dActive & state)
				RTextFace(PLAIN);
			else
				RTextFace(DIM);
			nRect = RTextBox(text, 0, &r, TEJUSTCENTER, font);
			RTextFace(0);

			/* back it out for frame */
			RInsetRect(&r, -1, -1);
			break;

		case dText:
			RInsetRect(&r, -1, -1);
			REraseRect(&r);
			RInsetRect(&r, 1, 1);
			nRect = RTextBox( text, 0, &r, GetProperty(item, s_mode), font);
			RCopyRect(&r, &ur);

			break;

		case dIcon:
			RCopyRect(&r, &ur);
			DrawCel	(
						ResLoad(RES_VIEW, (int) GetProperty(item, s_view)), 
						GetProperty(item, s_loop), 
						GetProperty(item, s_cel), 
						&r, 
						-1	/* needed for warren GetProperty(item, s_priority), */
						);
			if (dFrameRect & state)
				RFrameRect(&r);
			break;

		case dEdit:
			/* ensure that minimum control area is clear */
			REraseRect(&r);
			RInsetRect(&r, -1, -1);
			RFrameRect(&r);
			RCopyRect(&r, &ur);
			RInsetRect(&r, 1, 1);
			nRect = RTextBox( text, 0, &r, TEJUSTLEFT, font);
			break;

		default:
			break;
	}

	/* mark a "selected" control */
	if (dSelected & state)
		switch (type) {
			case dEdit:
				oldFont = GetFont();
				RSetFont(font);
				DrawCursor(&r, text, GetProperty(item, s_cursor));
				RSetFont(oldFont);
				break;
			case dSelector:
				break;
			case dIcon:
				break;
			default:
				RFrameRect(&r);
				break;
			}

	
	/* show the item's complete rectangle if NOT picNotValid */
	if (!noShowBits){
		ShowBits(&ur, VMAP);
	}
   return(nRect);
}



static void near	DrawSelector(item)
Obj	*item;
{
	int oldTop, i;
	RRect r;
	strptr str;
	word oldFont;
	int	font, fore, back;
	int	width;
	int	len;

	RCopyRect(GetPropAddr(item, s_nowSeen), &r);
	/* ensure that minimum control area is clear */
	REraseRect(&r);
	RInsetRect(&r, -1, -1);
	RFrameRect(&r);
	RTextBox("\x18", 0, &r, TEJUSTCENTER, 0);
	oldTop = r.top;
	r.top = r.bottom - 10;
	RTextBox("\x19", 0, &r, TEJUSTCENTER, 0);
	r.top = oldTop;
	RInsetRect(&r, 0, 10);
	RFrameRect(&r);
	RInsetRect(&r, 1, 1);

	oldFont = GetFont();
	RSetFont(font = GetProperty(item, s_font));

	/* get colors for inverting */
	back = rThePort->bkColor;
	fore = rThePort->fgColor;

	/* ready to draw contents of selector box */
	r.bottom = r.top + GetPointSize();
	str = Native(GetProperty(item, s_topString));
	width = GetProperty(item, s_x);
	for (i = 0; i < GetProperty(item, s_y); i++) {
		REraseRect(&r);
		if (*str) {

			RMoveTo(r.left, r.top);
			/*	allow for both fixed length and null-terminated strings
				(this relies on the fact that the entire array of selector
				strings must be terminated with a null
			*/
			len = strlen(str);
			RDrawText(str, 0, width < len ? width : len, font, fore);
			/* invert current selection line */
			if (str == Native(GetProperty(item, s_cursor))
				 && (GetProperty(item, s_type) != dScroller)
				)
				RInvertRect(&r);

			/* now set them back regardless */
			PenColor(fore);
			RBackColor(back);

			/* advance pointer to start of next string */
			str += GetProperty(item, s_x);
		}
		ROffsetRect(&r, 0, GetPointSize());
	}

	/* restore ports current font */
	RSetFont(oldFont);
}



void
RGlobalToLocal(mp)	/* make this global coord local */
RPoint *mp;
{
	mp->h -= rThePort->origin.h;
	mp->v -= rThePort->origin.v;
}



void
RLocalToGlobal(mp)	/* make this local coord global */
RPoint *mp;
{
	mp->h += rThePort->origin.h;
	mp->v += rThePort->origin.v;
}



static void near	TextEdit(item, evt)
Obj	*item, *evt;
{
	int			cursor;
	RRect			box;
	REventRecord	theEvent;

	/* get properties into locals */
	RCopyRect(GetPropAddr(item, s_nowSeen), &box);
	ObjToEvent(evt, &theEvent);

	cursor = EditText	(	&box,
								Native(GetProperty(item, s_text)),
								GetProperty(item, s_cursor),
								GetProperty(item, s_max),
								&theEvent
							);
	
	SetProperty(item, s_cursor, cursor);
}



int
EditText(box, text, cursor, max, evt)
RRect				*box;
strptr			text;
int				cursor, max;
REventRecord		*evt;
{
	RPoint mp;
	int	msg;
	int	oldCursor, i;
	bool	delete, changed;
	int	textLen;

	oldCursor = cursor;
	changed = FALSE;
	delete = FALSE;
	textLen = strlen(text);

	switch(evt->type) {
		case keyDown:
			switch (msg = evt->message) {
				case HOMEKEY:
					/* beginning of line */
					cursor = 0;
					break;

				case ENDKEY:
					/* end of line */
					cursor = textLen;
					break;

				case CLEARKEY:	/* control C */
					/* clear line */
					cursor = 0;
					*text = 0;
					changed = TRUE;
					break;
					
				case BS:
					/* destructive backspace */
					delete = TRUE;
					if (cursor)
						--cursor;
					break;

				case LEFTARROW:
					/* non-destructive backspace */
					if (cursor)
						--cursor;
					break;
		
				case DELETE:
					/* delete at cursor */
					if (cursor != textLen)
						delete = TRUE;
					break;
		
				case RIGHTARROW:
					if (cursor < textLen)
						++cursor;
					break;

				default:
					if (msg >= ' ' && msg < 257) {
						/* insert this key and advance cursor
						** if we have room in buffer AND we won't try to
						** draw outside of our enclosing rectangle
						*/
						if (
							textLen + 1 < max &&
							RCharWidth((char) msg) + RStringWidth(text) < (box->right - box->left)
							)
							{
							changed = TRUE;
							/* shift it up one */
							for(i = textLen ; i >= cursor; i--)
								*(text + i + 1) = *(text + i);
							*(text + cursor) = (char) msg;
							++(cursor);
							}
						}
					break;
				}
		
			/* if delateAt, we delete the character at cursor */
			if (delete) {
				changed = TRUE;
				/* collapse the string from cursor on */
				for (i = cursor ; i < textLen; i++)
					*(text + i) = *(text + i + 1);
				}

			break;

		case mouseDown:
			/* move cursor to closest character division */
			mp.h = evt->where.h;
			mp.v = evt->where.v;

			if (RPtInRect(&mp, box))
				for (
						cursor = textLen;
						cursor && (box->left + RTextWidth(text, 0, cursor, GetFont()) - 1 > mp.h) ;
						--cursor
					 )
						;
			break; 												
		}
	

	if (changed) {
		/* if we have changed we redraw the entire field in the text box */
		EraseCursor();
		REraseRect(box);
		RTextBox(text, 0, box, TEJUSTLEFT, -1);
		ShowBits(box, VMAP);
		DrawCursor(box, text, cursor);
		}
	else if (oldCursor == cursor)
		/* cursor is in the same place -- keep flashing */
		FlashCursor();
	else {
		/* cursor has moved -- ensure it is on at new position */
		EraseCursor();
		DrawCursor(box, text, cursor);
		}

	return (cursor);
}
