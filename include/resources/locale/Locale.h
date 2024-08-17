#pragma once

enum {
	LOC_LANGUAGE_NAME,
	LOC_LANGUAGE_CODE,
	LOC_COUNT
};

typedef u16 LocaleString;

const char* Locale_Get(LocaleString id);