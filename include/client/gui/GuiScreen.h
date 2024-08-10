#pragma once

typedef struct {
	void (*OnInit)();
	void (*OnAwake)();
	void (*OnDeinit)();
	void (*OnUpdate)();
	void (*OnDrawUp)();
	void (*OnDrawDown)();
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

extern Screen *currentScreen;

void GuiScreen_SetScreen(u8 screenIdx);
void GuiScreen_Tick();
void GuiScreen_DrawUp();
void GuiScreen_DrawDown();