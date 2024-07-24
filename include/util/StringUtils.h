#pragma once

#include "util/Paths.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_GETBASE(x) #x
#define STRING_GET(x) STRING_GETBASE(x)

static inline const char* String_ParsePackName(const char* packname, const char* subpath, const char* name) {
	u16 len = strlen(PATH_PACKS) + strlen(packname) + strlen(subpath) + strlen(name) + 3;

	char* result = (char*)malloc(len);

	snprintf(result, len, "%s%s/%s/%s", PATH_PACKS, packname, subpath, name);
	return result;
}