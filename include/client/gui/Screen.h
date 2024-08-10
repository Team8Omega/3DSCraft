#pragma once

typedef struct {
	void (*Init)();
	void (*Awake)();
	void (*Deinit)();
	void (*Tick)();
	void (*DrawUp)();
	void (*DrawDown)();
} Screen;

enum
{
	SCREEN_NONE = 0,
	SCREEN_TITLE,
	SCREEN_PAUSE,
	SCREEN_SELECTWORLD,
	SCREEN_CREATEWORLD,
	SCREEN_CONFIRMDELETION,
	SCREEN_COUNT
};
typedef u8 ScreenIndex;

extern Screen *currentScreen;
extern ScreenIndex currentScreenIdx;

void Screen_SetScreen(ScreenIndex screenIdx);
void Screen_Tick();
void Screen_DrawUp();
void Screen_DrawDown();