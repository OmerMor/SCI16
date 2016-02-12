#include "ctype.h"

#include "string.h"
#include "types.h"


#define euroToLower(c) euroToLowerTbl[(int)c-128]

static uchar near euroToLowerTbl[]=
{
	/* this array maps European characters to their lower case version
	   (lower case chars map to themselves)
	   euroToLowerTbl[(int)c-128] is lower case version of c
	   Use euroToLower(c) macro defined above.
	   NOTE: ASCII does not provide upper case versions of all lower case
	   European characters.
	*/
	135	/* 128 = C, cedille €*/
	,129
	,130
	,131
	,132
	,133
	,134
	,135
	,136
	,137
	,138
	,139
	,140
	,141
	,132	/* 142 = dotted A Ž */
	,134	/* 143 = accented A  */
	,130	/* 144 = accented E  */
	,145
	,145	/* 146 = AE ’ */
	,147
	,148
	,149
	,150
	,151
	,152
	,148	/* 153 = dotted O ™ */
	,129	/* 154 = dotted U š */
	,155
	,156
	,157
	,158
	,159
	,160
	,161
	,162
	,163
	,164
	,164	/* 165 = N~ ¥ */
};


bool
islower(uchar c)
{
	return
		(c >= 'a' && c <= 'z')
		||
		(iseuro(c) && (euroToLower(c) == c))
		||
		iskana(c)
		||
		isgreek(c);
}

bool
isupper(uchar c)
{
	return
		(c >= 'A' && c <= 'Z')
		||
		(iseuro(c) && (euroToLower(c) != c));
}

char
_tolower(uchar c)
{
	if (c >= 'A' && c <= 'Z')
		return c-'A'+'a';
	else if (iseuro(c))
		return euroToLower(c);
	else
		return c;
}

char
_toupper(uchar c)
{
	if (c >= 'a' && c <= 'z')
		return c - 'a'+'A';
	else
		/* we could do upcase conversion of Euorpean characters,
			but it seems too much of a pain, soooo....
		*/
		return c;
}
