#include "Fuji.h"

#if MF_TRANSLATION == NULL

#include "MFTranslation.h"

MFLanguage MFTranslation_GetDefaultLanguage()
{
	// the Null device doesnt know anything about languages ;)
	return MFLang_English;
}

#endif
