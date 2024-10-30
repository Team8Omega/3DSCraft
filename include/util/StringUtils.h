#pragma once

#include "util/Paths.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_GETBASE(x) #x
#define STRING_GET(x) STRING_GETBASE(x)

#define PATH_SIZE 256

static inline const char* String_ParseTextureNamePrefix(const char* folder, const char* name, const char* prefix) {
	size_t len = strlen(name) + strlen(prefix) + 13;

	char* path = (char*)malloc(len);

	snprintf(path, len, "%s/%s%s.png", folder, name, prefix);
	return path;
}
static inline char* String_ParsePackName(const char* packname, const char* subpath, const char* name) {
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
static inline char* String_RemoveSuffix(const char* path, const char* type) {
	size_t len = strlen(path) - strlen(type) + 1;

	char* nPath = (char*)malloc(len);

	strncpy(nPath, path, len);
	nPath[len] = '\0';

	return nPath;
}
static inline char* String_AddSuffix(const char* path, const char* type) {
	size_t len = strlen(path) + strlen(type) + 1;

	char* nPath = (char*)malloc(len);

	strcpy(nPath, path);
	strcat(nPath, type);
	nPath[len] = '\0';

	return nPath;
}
static inline u32 String_Hash(const char* in) {
	u32 hash = 5381;

	for (unsigned int i = 0; in[i]; ++i)
		hash = ((hash << 5) + hash) + in[i]; /* hash * 33 + c */

	return hash;
}
