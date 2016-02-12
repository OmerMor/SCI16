/*	STRING
	******
	String functions (standard C issue and user-defined strings).
	by Jeff Stephenson
	Multilingual conversion by Pablo Ghenis, April 1989
 
*** IMPORTANT!!! ***

Always include AFTER all standard include files, so that in case 
of redefinitions this file wins. This WILL happen since this file includes 
expressions that normally are found in stdlib.h, ctype.h, etc.
 
Identical versions of this string.c and string.h  should be used to
compile SCI, SC, VC, DC, VCPP and any other SCI-related programs.
If these programs are kept in separate directories make sure to migrate
changes to all others.
Recommendation: keep masters in SCI directory, copy as needed
to others.

**************************************************************************/

#ifdef SC

#define NeedPtr(n) Allocate(n)
#include	"etc.h"						/* Allocate is in SC's etc.c */

#else
#ifdef SCI

#include	"memmgr.h"					/* NeedPtr is in SCI's memmgr.c */

#else

#define NeedPtr(n) malloc(n)
#include	"stdlib.h"					/* malloc is in stdlib library */

#endif /* ifdef SCI */
#endif /* ifdef SC */


#include	"string.h"
#include	"ctype.h"




global uint strlen(s)
/* Return the length of the string pointed to by 's'.
 */
strptr	s;
{
	strptr	t;

	t = s;
	while (*t++)
		;
	return (t - s - 1);
}



global strptr strcpy(d, s)
strptr	s, d;
{
	strptr	ptr = d;

	while (*d++ = *s++)
		;

	return (ptr);
}



global strptr strncpy(d, s, n)
strptr	s, d;
		int		n;
{
	strptr	ptr = d;

	while ((*d++ = *s++) && --n)
		;
	*d++ = '\0';

	return (ptr);
}



global strptr strdup(s)
strptr	s;
{
	return (strcpy((strptr)NeedPtr(strlen(s) + 1), s));
}



global strptr strcat(s, t)
strptr	s, t;
{
	strptr	ptr = s;

	while (*s)
		++s;
	while (*s++ = *t++)
		;

	return (ptr);
}



global strptr strncat(d, s, n)
	strptr	s, d;
	int		n;
	{
	strptr	ptr = d;	/* save pointer to destination */

	while (*d) ++d;	/* find end of destination */
	strncpy(d, s, n);	/* copy up to n chars from source into end */
	return (ptr);		/* return pointer to destination string */
	}
	
global strptr strbcat(s, t, n)
strptr	s, t;
int		n;
{
	strptr	ptr = s;

	--n;					/* account for null termination */
	while (*s) {
		++s;
		--n;
		}
	while ((n-- > 0) && *t)
		*s++ = *t++
		;
	*s = '\0';
	
	return (ptr);
}



global int strcmp(s, t)
strptr	s, t;
{
	for ( ; *s != '\0' && *s == *t ; ++s, ++t)
		;
	return (*s - *t);
}



global int strncmp(s, t, n)
strptr	s, t;
int		n;
{
	for ( ; n && *s != '\0' && *s == *t ; ++s, ++t, --n)
		;
	return (n ? *s - *t : 0);
}







global strptr reverse(str)
strptr	str;
/* Reverse the string 'str' in place.
 */
{
			char	temp;
	strptr	s1, s2;

	s1 = str;
	s2 = str + strlen(str) - 1;
	while (s1 < s2) {
		temp = *s1;
		*s1++ = *s2;
		*s2-- = temp;
		}

	return (str);
}




global strptr strlwr(s)
strptr	s;
{
	strptr	t;

	for (t = s ; *t != '\0' ; ++t)
		*t = tolower(*t);

	return (s);
}



global bool IsPrintStr(s)
strptr	s;
{
	if (!s)
		return (FALSE);

	for ( ; *s ; ++s)
		if (*s < ' ' || *s > 0x7e)
			return (FALSE);

	return (TRUE);
}


global bool strtrn(ins, inpatt, outpatt, outs)
	strptr	ins, inpatt, outpatt, outs;
   {
   /*************************************************************
      
   usage: replace suffix or prefix in a string  
   
   strtrn("leaves","*ves","*f",rootstr);
      returns TRUE and sets rootstr to "leaf"
      
   strtrn("going","*ves","*f",rootstr);
      fails
      
   *************************************************************/
   
   int i;
   char before[30], star[30];
   
   /* match before *, build "before" string*/
   
   for(;*ins && *inpatt!='*';)         /* match prefix to * */  
      {
      if (*inpatt++!=*ins++)
         return FALSE;
      }
      
   for(i=0;*outpatt && *outpatt!='*';)  /* build new prefix */
      before[i++]=*outpatt++;
   before[i]='\0';
   
   
   /* skip * and build "star" string */
   
   if (!(*inpatt++=='*' && *outpatt++=='*'))
      return FALSE;
      
   for(i=0;*ins;)
      {
      if (strcmp(ins,inpatt)==0)        /* found suffix */
         break;
      star[i++]=*ins++;                  /* build "star" from "ins" */
      }
   star[i]='\0';
   
   /* match after *, "after" string is what's left of outpatt */
   
   if (strcmp(ins,inpatt)!=0)        
      return FALSE;
   
   
   /* finally, stuff it all into "outs" string parameter */
   
   *outs='\0';
   
   strbcat(outs,before,30);
   strbcat(outs,star,30);
   strbcat(outs,outpatt,30);
   
   return TRUE;
   }

strptr strchr(s,c)
	strptr s;
	char c;
/* standard c library function
*/	
{
	while(*s && (*s != c)) ++s;
	return ((*s && *s == c) ? s : NULL);
}

global strptr strstr(s1, s2)
strptr s1,s2;
{
	char *tmp1, *tmp2;
	
	for(; *s1; s1++) {
		for(tmp1=s1, tmp2=s2; *tmp1 == *tmp2 && *tmp2 != '\0'; tmp1++, tmp2++);
		if (*tmp2 == '\0') break;	/* we have a match! */
	}

	return (*s1 ? s1 : NULL);
}

