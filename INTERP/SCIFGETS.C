/*	scifgets.c		Our own personal version of fgets()
*/

#include "scifgets.h"
#include "io.h"
#include "types.h"

global strptr sci_fgets(str, len, fd)
strptr	str;
int		len;
int		fd;
{
	int		count = 0;
	char		c;
	char*		cp = str;
	
	--len;	/*	account for trailing	0	*/
	while (count < len) {
		if (read(fd, &c, 1) <= 0)
			break;
		count++;
		if (c == '\n')
			break;
		if (c == '\r')
			continue;
		*cp++ = c;
	};
	*cp = '\0';
	
	return count ? str : NULL;
}
