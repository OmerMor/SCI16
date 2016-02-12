/* FARDATA
 * Routines to handle reading data from hunk to heap buffers.
 */


#include	"fardata.h"
#include	"types.h"
#include	"resource.h"
#include	"pmachine.h"
#include	"memmgr.h"

global int GetFarData(moduleNum, entryNum, buffer)
int		moduleNum;
int		entryNum;
memptr	buffer;
{
	Handle	dh;
	Hunkptr	dp;
	memptr	sp;
	int		ofs, length, i;

	/* Get a handle to the appropriate resource.
	 */
	dh = ResLoad(RES_VOCAB, moduleNum);
	dp = *dh;

	/* Value check the requested entryNum (first word of resource contains
	 * the maximum directory entry).
	 */
	if (entryNum > *dp + *(dp+1)*256)
		return (0);

	/* Get a pointer to the data by looking up its offset in the
	 * directory.
	 */
	entryNum = 2 * (entryNum + 1);
	ofs = *(dp + entryNum) + *(dp + entryNum + 1) * 256;
	dp += ofs;

	/* Get the length of the data.
	 */
	length = *dp + *(dp+1) * 256;

	/* Copy the data to its destination.
	 */
	for (sp = buffer, dp += 2, i = length ;
		i > 0 ;
		++sp, ++dp, --i
		 )
		*sp = *dp;

	/* Return the length of the data.
	 */
	return (length);
}

strptr
GetFarStr(int moduleNum, int entryNum, strptr buffer)
{
	int	length;

	length = GetFarData(moduleNum, entryNum, (memptr) buffer);
	buffer[length] = '\0';

	return length ? buffer : NULL;
}

void
KGetFarText(word* args)
{
	acc = Pseudo(
		GetFarText(*(args+1), *(args+2), (strptr) Native(*(args+3))));
}



global strptr GetFarText(module, n, buffer)
uint		module;
uint		n;
strptr	buffer;
{
	Handle	lp;
	Hunkptr	hp;
	strptr	bp;

	/* Find the script, get the handle to its far text, find the
	 * string in the script, then copy the string to the buffer.
	 */
	bp = buffer;
	if (!(lp = ResLoad(RES_TEXT, module)))
		/*	shouldn't get back here...but just in case	*/
		*buffer = '\0';
	else {
		hp = *lp;
		/* Scan for the nth string.
		 */
		while (n-- != 0) {
			while (*hp++)
				;
			}
		/* Then copy the string.
		 */
		while (*hp)
			*bp++ = *hp++;
		*bp = '\0';
		}
		
	return (buffer);
}

