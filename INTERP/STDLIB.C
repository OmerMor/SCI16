#include	"stdlib.h"

#include	"types.h"
#include	"ctype.h"
#include	"string.h"

global int atoi(str)
strptr	str;
{
	int		n;
	char	c;
	int		sign;

	/* Scan off spaces
	 */
	for ( ; isspace(*str); str++)
		;

	/* Initialize.
	 */
	sign = 1;
	n = 0;

	/* Check for a negative number.
	 */
	if (*str == '-') {
		sign = -1;
		++str;
		}

	if (*str != '$') {
		/* A decimal number.
		 */
		while (isdigit(*str))
			n = 10*n + *str++ - '0';
		}
	else {
		/* A hex number.
		 */
		++str;
		forever {
			c = lower(*str++);
			if (isdigit(c))
				c = c - '0';
			else if (c >= 'a' && c <= 'f')
				c = c + 10 - 'a';
			else
				break;
			n = 16 * n + c;
			}
		}

	return (sign * n);
}



global strptr itoa(n, sp, radix)
int		n, radix;
strptr	sp;
{
	if (n >= 0)
		ultoa((ulong) n, sp, radix);
	else {
		*sp = '-';
		ultoa((ulong) (-n), sp+1, radix);
		}

	return (sp);
}




global strptr ultoa(n, sp, radix)
ulong	n;
strptr	sp;
int		radix;
{
	strptr	s;
	char		c;

	s = sp;
	do	{
		c = (char) (n % radix);
		*s++ = c + ((c > 9)?  'a'-10 : '0');
	} while ((n /= radix) > 0);
	*s = '\0';

	return (reverse(sp));
}
