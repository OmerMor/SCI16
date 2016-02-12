#include "language.h"

#include "types.h"
#include "string.h"
#include "pmachine.h"
#include "selector.h"
#include "start.h"
#include "stdio.h"
#include "object.h"
#include	"errmsg.h"

global int language=ENGLISH;

global int GetLanguage()
{
	if (theGame) SetProperty(theGame, s_printLang, language);
	switch (language)
		{
		case ENGLISH:
		case JAPANESE:
		case GERMAN:
		case FRENCH:
		case SPANISH:
		case ITALIAN:
		case PORTUGUESE:
			break;
		default:
			Panic(E_BAD_LANG, language);
		}
		
	return language;
}
