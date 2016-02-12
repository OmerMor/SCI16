#include	"stdio.h"

#include	"types.h"
#include	"ctype.h"
#include	"string.h"
#include	"stdlib.h"

#ifdef	LINT_ARGS
	static strptr	near CopyString(strptr, strptr);
#else
	static strptr	near CopyString();
#endif

static int	near justified, fieldWidth;
static bool	near leadingZeros;

#define	RIGHT		0
#define	CENTER	1
#define	LEFT		2

void
puts(strptr str)
{
	/*	duplicated here, because SCI has none */
	write(1, str, strlen(str));
	write(1, "\r\n", 2);
}

int
sprintf(char* str, char* fp, ...)
{
	va_list	ap;
	int		rc;

	va_start(ap, fp);
	rc = vsprintf(str, fp, ap);
	va_end(ap);
	return rc;
}

int
vsprintf(char* str, char* fp, va_list ap)
{
	strptr	sp;
	char		theStr[20];

	sp = str;
	while (*fp) {
		if (*fp != '%')
			*sp++ = *fp++;
		else {
			++fp;
			fieldWidth = 0;
			leadingZeros = FALSE;

			/* Check for justification in a field.
			 */
			switch (*fp) {
				case '-':
					++fp;
					justified = LEFT;
					break;

				case '=':
					++fp;
					justified = CENTER;
					break;

				default:
					justified = RIGHT;
				}

			/* Check for a field width.
			 */
			if (isdigit(*fp)) {
				leadingZeros = (*fp == '0') && (justified == RIGHT);
				fieldWidth = atoi(fp);
				while (isdigit(*fp))
					++fp;
				}

			switch (*fp++) {
				case 'd':
					itoa(va_arg(ap, int), theStr, 10);
					sp = CopyString(sp, theStr);
					break;

				case 'u':
					ultoa((ulong) va_arg(ap, uint), theStr, 10);
					sp = CopyString(sp, theStr);
					break;

				case 'U':
					ultoa(va_arg(ap, ulong), theStr, 10);
					sp = CopyString(sp, theStr);
					break;

				case 'x':
					ultoa((ulong) va_arg(ap, uint), theStr, 16);
					sp = CopyString(sp, theStr);
					break;

				case 'X':
					ultoa(va_arg(ap, ulong), theStr, 16);
					sp = CopyString(sp, theStr);
					break;

				case 'c':
					theStr[0] = (char) va_arg(ap, int);
					theStr[1] = '\0';
					sp = CopyString(sp, theStr);
					break;

				case 's':
					sp = CopyString(sp, va_arg(ap, strptr));
					break;

				default:
					*sp++ = *(fp-1);
				}
			}
		}

	*sp = '\0';

	return (sp - str);
}




static strptr near CopyString(sp, tp)
strptr	sp, tp;
{
	char		c;
	strptr	fp;
	int		fw, sLen;

	/* Get the length of the string to print.  If the length is greater
	 * than the field width, set the field width to 0 (i.e. print the
	 * entire string).
	 */
	sLen = strlen(tp);
	if (sLen >= fieldWidth)
		fieldWidth = 0;

	/* If there is a field width, clear the field to the appropriate
	 * character, point sp to the place at which to start copying the
	 * string, and point fp at the end of the string (do the latter
	 * even if there is no field width).
	 */
	if (fieldWidth > 0) 
	{
		c = (char) ((leadingZeros)? '0' : ' ');

		for (fw = fieldWidth, fp = sp ; fw > 0 ; --fw, ++fp)
			*fp = c;

		switch (justified)	
		{
			case RIGHT:
				sp += fieldWidth - sLen;
				break;

			case CENTER:
				sp += ((fieldWidth - sLen) / 2);
				break;
		}
	}
	else
		fp = sp + sLen;

	while (*tp)
		*sp++ = *tp++;

	return (fp);
}
