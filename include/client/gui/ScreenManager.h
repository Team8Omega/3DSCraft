#pragma once

typedef struct {
	void (*OnInit)();
	void (*OnAwake)();
	void (*OnDeinit)();
	void (*OnUpdate)();
	void (*OnDrawUp)();
	void (*OnDrawDown)();
} Screen;

extern Screen *currentScreen, *previousScreen;

void ScreenManager_SetScreen(Screen* screen);
void ScreenManager_SetPrevious();
void ScreenManager_Update();
void ScreenManager_DrawUp();
void ScreenManager_DrawDown();