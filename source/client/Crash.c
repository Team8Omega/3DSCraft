#include "client/Crash.h"

#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <3ds.h>
#include <citro3d.h>

#include "Globals.h"
#include "client/Game.h"
#include "util/Paths.h"

static char sharedReason[512] = { 0 };
static CRASH_TYPE sharedType  = 0;

static void crash_extInfo(FILE* file, CRASH_TYPE type);

void Crash_Check() {
	if (sharedReason[0] != 0)
		Crash(sharedType, (const char*)sharedReason);
}

void Crash(CRASH_TYPE type, const char* reason, ...) {
	aptSetHomeAllowed(false);

	if (threadGetHandle(threadGetCurrent()) != gThreadMain) {
		va_list vl;
		va_start(vl, reason);
		vsnprintf((char*)sharedReason, sizeof(sharedReason), reason, vl);
		sharedType = type;
		va_end(vl);
		return;
	}

	if (gfxGetFramebuffer(0, 0, 0, 0) == NULL)
		gfxInitDefault();

	gfxSet3D(false);
	consoleInit(GFX_TOP, NULL);

	va_list vl;
	va_start(vl, reason);
	vprintf(reason, vl);
	va_end(vl);

	va_start(vl, reason);
	FILE* f = fopen(PATH_ROOT "crash.txt", "w");
	vfprintf(f, reason, vl);
	crash_extInfo(f, type);
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

#define FPRINT(file, str, ...)                                                                                                             \
	do {                                                                                                                                   \
		printf(str, ##__VA_ARGS__);                                                                                                        \
		fprintf(file, str, ##__VA_ARGS__);                                                                                                 \
	} while (0)

static void crash_extInfo(FILE* file, CRASH_TYPE type) {
	FPRINT(file, "\n\n");
	switch (type) {
		case CRASH_ALLOC:
			FPRINT(file, "Linear Free: %lu/%lu kB\nHeap Free: %lu/%lu kB", linearSpaceFree() >> 10, envGetLinearHeapSize() >> 10,
				   (envGetHeapSize() - mallinfo().uordblks) >> 10, envGetHeapSize() >> 10);
			break;
		default:
			break;
	}
}