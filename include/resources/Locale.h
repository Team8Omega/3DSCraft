#pragma once

enum {
	LOC_LANGUAGE_NAME,
	LOC_LANGUAGE_CODE,
	LOC_MENU_SINGLEPLAYER,
	LOC_MENU_QUIT,
	LOC_COUNT
};

typedef u16 LocaleString;

extern char* localeGet[LOC_COUNT];