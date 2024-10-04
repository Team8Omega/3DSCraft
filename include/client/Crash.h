#pragma once

enum {
	CRASH_DEFAULT = 0,
	CRASH_ALLOC
};
typedef u8 CRASH_TYPE;

void Crash(CRASH_TYPE type, const char* reason, ...);
void Crash_Check();