#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro3d.h>

#include "Globals.h"
#include "client/Crash.h"
#include "client/Game.h"
#include "util/Paths.h"

void Crash(const char* reason, ...) {
	if (gfxGetFramebuffer(0, 0, 0, 0) == NULL)
		gfxInitDefault();

	consoleInit(GFX_TOP, NULL);

	va_list vl;
	va_start(vl, reason);
	vprintf(reason, vl);

	FILE* f = fopen(PATH_ROOT "crash.txt", "w");
	vfprintf(f, reason, vl);
	fclose(f);

	va_end(vl);

	if (gIsRunning())
		printf("\n\nIngame error, press start to exit\n");
	else
		printf("\n\n");
#ifdef TEST_UNCRASH
	printf("Running with debug, may press select to continue");
#endif
	while (true) {
		gspWaitForVBlank();

		hidScanInput();

		if (hidKeysDown() & KEY_START)
			exit(EXIT_FAILURE);

#ifdef TEST_UNCRASH
		if (hidKeysDown() & KEY_SELECT)
			break;
#endif
	}
	gfxExit();
	gfxInitDefault();
	gfxSet3D(true);
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
