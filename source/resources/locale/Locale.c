#include "resources/locale/Locale.h"

extern char* localeStrings[LOC_COUNT];

const char* Locale_Get(LocaleString id) {
	return localeStrings[id];
}