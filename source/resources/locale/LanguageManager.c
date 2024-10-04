#include "resources/locale/LanguageManager.h"

#include <mpack/mpack.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "client/Crash.h"
#include "resources/Locale.h"
#include "util/Paths.h"

char* localeGet[LOC_COUNT];

extern const char* localeKeys[LOC_COUNT];

#define defaultLang "en_us"

void LanguageManager_Init() {
	const char* language = defaultLang;

	LanguageManager_Load(language);
}
void LanguageManager_Deinit() {
	for (u16 i = 0; i < LOC_COUNT; ++i) {
		if (localeGet[i] == NULL)
			continue;

		free((void*)localeGet[0]);
		localeGet[0] = NULL;
	}
}

#define save_get(root, key, charPtr) mpack_node_copy_utf8_cstr(mpack_node_map_cstr(root, key), charPtr, 512);

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
		Crash(0, "Language file for english (lang/en_us.mp) is missing!");
		return;
	}

	LanguageManager_Deinit();

	mpack_tree_t levelTree;
	mpack_tree_init_file(&levelTree, filename, 0);
	mpack_node_t root = mpack_tree_root(&levelTree);

	char buffer[512];
	for (u16 i = 0; i < LOC_COUNT; ++i) {
		save_get(root, localeKeys[i], buffer);
		localeGet[i] = malloc((strlen(buffer) + 1) * sizeof(char));
		strcpy(localeGet[i], buffer);
	}

	mpack_error_t err = mpack_tree_destroy(&levelTree);
	if (err != mpack_ok) {
		Crash(0, "Mpack error %d while loading language %s", err, langCode);
	}
}