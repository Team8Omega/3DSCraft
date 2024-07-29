//
// Created by Elias on 27.07.2024.
//
#include "client/Crash.h"
#include "client/gui/DebugUI.h"
#include "client/language/LanguageManager.h"
#include "util/StringUtils.h"
#include <mpack/mpack.h>

void test() {
	// Load the file
	FILE *file = fopen(String_ParsePackName("minecraft", "language", "en_us.mp"), "rb");
	if (!file) {
		// Crash("Failed to open file for language, testing.");
		return;
	}

	// Get the file size
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Allocate memory to read the file
	char *data = (char *)malloc(size);
	if (!data) {
		Crash("Failed to allocate memory");
		fclose(file);
		return;
	}

	// Read the file into the buffer
	if (fread(data, 1, size, file) != size) {
		Crash("Failed to read file");
		free(data);
		fclose(file);
		return;
	}

	fclose(file);

	// Initialize an mpack reader
	mpack_reader_t reader;
	mpack_reader_init_data(&reader, data, size);

	// Read the map
	mpack_expect_map(&reader);
	while (mpack_reader_remaining(&reader, NULL)) {
		char key[256];
		mpack_expect_cstr(&reader, key, sizeof(key));

		if (strcmp(key, "name") == 0) {
			char value[256];
			mpack_expect_cstr(&reader, value, sizeof(value));
			Crash("name: %s", value);
		} else if (strcmp(key, "age") == 0) {
			int value = mpack_expect_int(&reader);
			Crash("age: %d", value);
		} else if (strcmp(key, "isMale") == 0) {
			bool value = mpack_expect_bool(&reader);
			Crash("isMale: %s", value ? "true" : "false");
		} else {
			mpack_discard(&reader);
		}
	}

	mpack_done_map(&reader);

	// Check for errors
	if (mpack_reader_destroy(&reader) != mpack_ok) {
		fprintf(stderr, "An error occurred decoding the data!\n");
	}

	// Clean up
	free(data);
}