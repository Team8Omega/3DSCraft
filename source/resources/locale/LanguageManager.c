#include "resources/locale/LanguageManager.h"

#include <mpack/mpack.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "client/Crash.h"
#include "resources/locale/Locale.h"
#include "util/Paths.h"

char* localeStrings[LOC_COUNT];

static const char* localeKeys[LOC_COUNT] = {
	[LOC_LANGUAGE_NAME] = "language.name",
	[LOC_LANGUAGE_CODE] = "language.code",
};

#define defaultLang "en_us"

void LanguageManager_Init() {
	const char* language = defaultLang;

	LanguageManager_Load(language);
}
void LanguageManager_Deinit() {
	for (u16 i = 0; i < LOC_COUNT; ++i) {
		if (localeStrings[i] == NULL)
			continue;

		free((void*)localeStrings[0]);
		localeStrings[0] = NULL;
	}
}

#define mpack_get(root, key, charPtr) mpack_node_copy_utf8_cstr(mpack_node_map_cstr(root, key), charPtr, 512);

static void getLangFilename(char* buffer, const char* langCode) {
	strcpy(buffer, langCode);
	strcat(buffer, ".mp");
}

void LanguageManager_Load(const char* langCode) {
	chdir(PATH_PACKS);
	chdir(PACK_VANILLA);
	chdir("lang");

	char filename[10];
	getLangFilename(filename, langCode);

	if (access(filename, F_OK) == -1)
		getLangFilename(filename, defaultLang);

	if (access(filename, F_OK) == -1) {
		Crash("Language file for english (lang/en_us.mp) is missing!");
		return;
	}

	Crash("LANG: %s", filename);

	mpack_tree_t levelTree;
	mpack_tree_init_file(&levelTree, filename, 0);
	mpack_node_t root = mpack_tree_root(&levelTree);

	char buffer[512];
	for (u16 i = 0; i < LOC_COUNT; ++i) {
		mpack_get(root, localeKeys[i], buffer);
		localeStrings[i] = malloc((strlen(buffer) + 1) * sizeof(char));
		strcpy(localeStrings[i], buffer);
	}

	Crash("LANG: %s LEN: %d", localeStrings[0], strlen(localeStrings[0]));

	mpack_error_t err = mpack_tree_destroy(&levelTree);
	if (err != mpack_ok) {
		Crash("Mpack error %d while loading language %s", err, langCode);
	}
}