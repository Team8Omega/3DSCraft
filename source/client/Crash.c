#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro3d.h>

#include "Globals.h"
#include "client/Crash.h"
#include "client/Game.h"
#include "util/Paths.h"

static char sharedReason[512] = { 0 };

void Crash_Check() {
	if (sharedReason[0] != 0)
		Crash((const char*)sharedReason);
}

void Crash(const char* reason, ...) {
	aptSetHomeAllowed(false);

	if (threadGetHandle(threadGetCurrent()) != gThreadMain) {
		va_list vl;
		va_start(vl, reason);
		vsnprintf((char*)sharedReason, sizeof(sharedReason), reason, vl);
		return;
	}

	if (gfxGetFramebuffer(0, 0, 0, 0) == NULL)
		gfxInitDefault();

	gfxSet3D(false);

	consoleInit(GFX_TOP, NULL);

	va_list vl;
	va_start(vl, reason);
	vprintf(reason, vl);

	FILE* f = fopen(PATH_ROOT "crash.txt", "w");
	vfprintf(f, reason, vl);
	fclose(f);

	va_end(vl);

	if (gIsRunning()) {
		printf("\n\nIngame error, press start to exit\n");
#ifdef TEST_UNCRASH
		printf("Running with debug, may press select to continue");
#endif
	} else {
		printf("\n\n");
	}
	while (true) {
		gspWaitForVBlank();

		hidScanInput();

		if (hidKeysDown() & KEY_START) {
			gStop();
			break;
		}

#ifdef TEST_UNCRASH
		if (hidKeysDown() & KEY_SELECT) {
			gfxExit();
			gfxInitDefault();
			gfxSet3D(true);
			aptSetHomeAllowed(true);
			sharedReason[0] = 0;
			break;
		}
#endif
	}
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
