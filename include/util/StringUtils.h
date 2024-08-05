#pragma once

#include "util/Paths.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_GETBASE(x) #x
#define STRING_GET(x) STRING_GETBASE(x)

static inline const char* String_ParsePackName(const char* packname, const char* subpath, const char* name) {
	size_t len = strlen(PATH_PACKS) + strlen(packname) + strlen(subpath) + strlen(name) + 3;

	char* result = (char*)malloc(len);

	snprintf(result, len, "%s%s/%s/%s", PATH_PACKS, packname, subpath, name);
	return result;
}
static inline const char* String_ParseTextureName(const char* folder, const char* name) {
	size_t len = strlen(name) + 13;

	char* path = (char*)malloc(len);

	snprintf(path, len, "%s/%s.png", folder, name);
	return path;
}
static inline const char* String_ParseTextureNamePrefix(const char* folder, const char* name, const char* prefix) {
	size_t len = strlen(name) + strlen(prefix) + 13;

	char* path = (char*)malloc(len);

	snprintf(path, len, "%s/%s%s.png", folder, name, prefix);
	return path;
}