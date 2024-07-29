#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro3d.h>

#include "client/Crash.h"
#include "util/Paths.h"

void Crash(const char* reason, ...) {
	consoleInit(GFX_TOP, NULL);

	va_list vl;
	va_start(vl, reason);
	vprintf(reason, vl);

	FILE* f = fopen(PATH_ROOT "crash.txt", "w");
	vfprintf(f, reason, vl);
	fclose(f);

	va_end(vl);

	printf("\n\nFatal error, press start to exit\n");
#ifdef _DEBUG
	printf("Running with debug, may press select to continue");
#endif
	while (true) {
		gspWaitForVBlank();

		hidScanInput();

		if (hidKeysDown() & KEY_START)
			exit(EXIT_FAILURE);

#ifdef _DEBUG
		if (hidKeysDown() & KEY_SELECT)
			break;
#endif
	}
	gfxInitDefault();
}

void Log(const char* reason, ...) {
	va_list vl;
	va_start(vl, reason);
	vprintf(reason, vl);

	FILE* f = fopen(PATH_ROOT "Log.txt", "a");
	vfprintf(f, reason, vl);
	fclose(f);

	va_end(vl);
}
