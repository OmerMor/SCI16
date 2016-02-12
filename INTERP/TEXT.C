/*

** TEXT - Miscellaneous text routines.
**
*/
#include	"text.h"
#include	"types.h"
#include	"ctype.h"
#include	"grtypes.h"
#include	"resource.h"
#include	"string.h"
#include	"event.h"
#include	"language.h"
#include	"graph.h"
#include "dialog.h"
#include "stdio.h"
#include	"errmsg.h"
#include "memmgr.h"

uchar	textColors[MAXTEXTCOLORS];
uint 	textFonts[MAXTEXTFONTS];
uchar	lastTextColor;
uchar	lastTextFont;
word*	newRect;
word	rectIndex;

global void KTextColors(args)
argList;
{
	int	i;
	uchar	*color = textColors;

	for(i = 1; i <= argCount; i++)
		*color++ = (uchar)arg(i);
	lastTextColor = (uchar)argCount;
}

global void KTextFonts(args)
argList;
{
	int	i;
	uint	*font = textFonts;

	for(i = 1; i <= argCount; i++)
		*font++ = (uint)arg(i);
	lastTextFont = (uchar)argCount;
}


#define DEFWIDE	192
 
global void RTextSize(r, text, font, def)
	/* make r large enough to hold text */
RRect *r;
strptr text;
word font , def;
{

	word length, longest, count, height, oldFont, defaultFont;
	strptr str, first;

	/* we are sizing this text in the font requested */
	oldFont = GetFont();

	if (font != -1){
		RSetFont(defaultFont = font);
	} else {
		defaultFont = oldFont;
	}

	r->top = 0;
	r->left = 0;

	if (def < 0){
		/* we don't want word wrap */
		r->bottom = GetHighest(text, strlen(text), defaultFont);
		r->right = RStringWidth(text);
	} else {
		if (!def){
			/* use default width */
			r->right = DEFWIDE;
		} else {
			r->right = def;
		}
//		do {
		/* get a local pointer to text */
			str = text;
			height = 0;
			longest = 0;
	 		while (*str){
				first = str;
				height += GetHighest(first, (count = GetLongest(&str, r->right, defaultFont)), defaultFont);
				length = RTextWidth(first , 0, count, defaultFont);
				if (length > longest)
					longest = length;
			}
			if (!def && r->right > longest){
				r->right = longest;
			}
			r->bottom = height;
//		} while (0);	/* r->right / 2 < r->bottom);	 test for ratio */
	}

	/* restore old font */
	RSetFont(oldFont);
}



global int GetLongest(str, max, defaultFont)	/* return count of chars that fit in pixel length */
strptr *str;
int max, defaultFont;
{

	strptr last, first;
	char c;
	word count = 0, lastCount = 0;

	first = last = *str;

	/* find a HARD terminator or LAST SPACE that fits on line */

	while(1){

		c = *(*str);
		if (c == 0x0d){
			if (*(*str + 1) == 0x0a)
				(*str)++;			/* so we don't see it later */
			if (lastCount &&  max < RTextWidth(first , 0, count, defaultFont)){
				*str = last;
				return(lastCount);
			} else {
				(*str)++;			/* so we don't see it later */
				return(count); 	/* caller sees end of string */
			}
		}
		if (c == LF){
			if ((*(*str + 1) == CR) && (*(*str + 2) != LF)) /*by Corey for 68k*/
				(*str)++;			/* so we don't see it later */
			if (lastCount &&  max < RTextWidth(first , 0, count, defaultFont)){
				*str = last;
				return(lastCount);
			} else {
				(*str)++;			/* so we don't see it later */
				return(count); 	/* caller sees end of string */
			}
		}

		if (c == '\0'){
			if (lastCount && max < RTextWidth(first , 0, count, defaultFont)){
				*str = last;
				return(lastCount);
			} else {
				return(count); 	/* caller sees end of string */
			}
		}


		if (c == SP){			/* check word wrap */
			if (max >= RTextWidth(first , 0, count, defaultFont)){
				last = *str;
				++last;		/* so we don't see space again */
				lastCount = count;
			} else {
				*str = last;
				// eliminate trailing spaces
				while (**str == ' ') {
					++(*str);
				}
				return(lastCount);
			}
		}

	/* all is still cool */
		++count;
		(*str)++;

		{

		/* we may never see a space to break on */
			if (!lastCount &&  RTextWidth(first, 0, count, defaultFont) > max){
				last += --count;
				*str = last;
				return(count);
         }
		}
	}
}




/* Search string for font control and return point size of tallest font */
global int GetHighest(str, cnt, defaultFont)
strptr str;
int cnt, defaultFont;
{
	int oldFont, pointSize, start = cnt - 2, setFont, newFont;

	oldFont = GetFont();
	pointSize = GetPointSize();

	while(cnt--){
		if (*str++ == CTRL_CHAR){
			// Hit control code: check for font control
			// If font control found, adjust pointSize if a taller font
			// is requested.

			if(*str == 'f') {
				if(!cnt--) break;
				str++;
				setFont = (cnt == start);	//If font control at start of line, set
													//pointSize even if smaller than default.
				if(*str == CTRL_CHAR) {
					RSetFont(defaultFont);
				} else {
					newFont = 0;
					while(cnt-- && (*str >= '0') && (*str <= '9')) {
						newFont *= 10;
						newFont += *str++ - '0';
					}
					if(!++cnt) break;
					RSetFont(textFonts[newFont]);
				}
				if(setFont || (pointSize < GetPointSize()))
					pointSize = GetPointSize();
			}
			if(!cnt--) break;
			while((*str++ != CTRL_CHAR) && cnt--) ;
		}
	}
	RSetFont(oldFont);
	return(pointSize);
}




global word *RTextBox(text, show, box, mode, font)	/* put the text to the box in mode requested */
strptr text;
word show, mode, font;
RRect *box;
{
	strptr first, str;
	word length, height = 0, wide, xPos, count, oldFont, curFont, pointSize;
	word defaultFont, defaultFore,i;

   rectIndex = 0;
   newRect = (word *) RNewPtr((((strlen(text)/7)*4+1)*sizeof(word)));

/* we are printing this text in the font requested */
	oldFont = GetFont();

	if (font != -1){
		RSetFont(defaultFont = font);
	} else {
		defaultFont = oldFont;
	}
	defaultFore = (uchar)rThePort->fgColor;

	wide = box->right - box->left;

	str = text;

	while (*str){
		first = str;
		curFont = GetFont();		// RTextWidth may change our font, so we must save it.
		pointSize = GetHighest(first, (count = GetLongest(&str, wide, defaultFont)), defaultFont);
		length = RTextWidth(first , 0, count, defaultFont);
		RSetFont(curFont);

	/* determine justification and draw the line */
		switch (mode){
			case TEJUSTCENTER:
				xPos = (wide - length) / 2;
				break;
			case TEJUSTRIGHT:
				xPos = (wide - length);
				break;
			case TEJUSTLEFT:
			default:
				xPos = 0;
		}
		RMoveTo(box->left + xPos, box->top + height);
		if (show)
			ShowText(first, 0, count, defaultFont, defaultFore);
		else
			RDrawText(first, 0, count, defaultFont, defaultFore);
		height += pointSize;
	}

/* restore old font */
	RSetFont(oldFont);
   for(i=0;i<rectIndex;i++)
   {
      newRect[i*4]   += rThePort->origin.h;
      newRect[i*4+1] += rThePort->origin.v;
      newRect[i*4+2] += rThePort->origin.h;
      newRect[i*4+3] += rThePort->origin.v;
   }
   newRect[rectIndex*4] = 0x7777;
   if (rectIndex == 0)
   {
      DisposePtr(newRect);
      newRect = NULL;
   }
   return(newRect);
}


global int RStringWidth(str)
strptr str;
{
	return(RTextWidth(str,0,strlen(str),GetFont()));
}





global void DrawString(str)	/* draw and show this string */
strptr str;
{
	RDrawText(str, 0, strlen(str), GetFont(), rThePort->fgColor);
}
	


global void ShowString(str)	/* draw and show this string */
strptr str;
{
	ShowText(str, 0, strlen(str), GetFont(), rThePort->fgColor);
}


global void RDrawText(str, first, cnt, defaultFont, defaultFore)
strptr str;
int first, cnt;
int defaultFont, defaultFore;
{
	char		code;
	int		param;
	strptr	last;
#ifdef DEBUG
	strptr	chkParam;
#endif
   word     TogRect;

   TogRect  = 0;

	str += first;
	last = str + cnt;

	while(str < last){
		if(*str == CTRL_CHAR) {
			str++;
			// Hit control code: do control function

			if((code = *str++) == CTRL_CHAR) {
				RDrawChar(CTRL_CHAR);		// No control code found -> want CTRL_CHAR
				continue;
			}

			if(*str == CTRL_CHAR) {
				// No parameter following control code
				str++;
				param = -1;
			} else {
				param = 0;
#ifdef DEBUG
				chkParam = str;
#endif
				while((*str >= '0') && (*str <= '9')) {
					param *= 10;
					param += (*str++) - '0';
				}
#ifdef DEBUG
				if(str == chkParam) {		//No number found
					Panic(E_TEXT_PARAM, code, *str);
				}
#endif
				while((str < last) && (*str++ != CTRL_CHAR)) ;
			}
			switch(code) {
            //rectangle
            case 'r':
               if (TogRect) {
                  newRect[rectIndex*4+2] = rThePort->pnLoc.h;
                  newRect[rectIndex*4+3] = rThePort->pnLoc.v + rThePort->fontSize;
                  rectIndex++;
               } else {
                  newRect[rectIndex*4]   = rThePort->pnLoc.h;
                  newRect[rectIndex*4+1] = rThePort->pnLoc.v;
               }
               TogRect = !TogRect;
               break;
  				//Color
				case 'c':
					if(param == -1) {
						//No param = set color to default color
						PenColor(defaultFore);
					} else {
						//Param = index in textColors table
						if(param < lastTextColor) {
							PenColor(textColors[param] & 0xff);
						}
#ifdef DEBUG
						else {
							Panic(E_TEXT_COLOR, param);
						}
#endif
					}
					break;
				//Font
				case 'f':
					if(param == -1) {
						//No param = set font to default
						RSetFont(defaultFont);
					} else {
						//Param = index in textFonts table
						if(param < lastTextFont) {
							RSetFont(textFonts[param]);
						}
#ifdef DEBUG
						else {
							Panic(E_TEXT_FONT, param);
						}
#endif
					}
					break;
#ifdef DEBUG
				default:
					Panic(E_TEXT_CODE, code);
					break;
#endif
			}
		} else {
			RDrawChar(*str++);
		}
	}
  
}



global void ShowText(str, first, cnt, defaultFont, defaultFore)
strptr str;
int first,cnt;
int defaultFont, defaultFore;
{
	RRect r;
	
	r.top = rThePort->pnLoc.v;
	r.bottom = r.top + GetHighest(str + first, cnt, defaultFont);
	r.left = rThePort->pnLoc.h;
	RDrawText(str, first, cnt, defaultFont, defaultFore);
	
	r.right = rThePort->pnLoc.h;
	ShowBits(&r, VMAP);
}
