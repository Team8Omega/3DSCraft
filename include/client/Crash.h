#pragma once

enum {
	CRASH_DEFAULT = 0,
	CRASH_ALLOC
};
typedef u8 CRASH_TYPE;

void Crash(const char* reason, ...);
void Crash_Ext(const char* reason, CRASH_TYPE type, ...);
void Crash_Check();