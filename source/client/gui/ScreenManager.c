#include "client/gui/ScreenManager.h"

#include "client/gui/screens/TitleScreen.h"

Screen *currentScreen = NULL, *previousScreen = NULL;

void ScreenManager_SetScreen(Screen* screen) {
	if (currentScreen != NULL && currentScreen->OnDeinit != NULL)
		currentScreen->OnDeinit();

	previousScreen = currentScreen;
	currentScreen  = screen;

	if (currentScreen != NULL) {
		if (currentScreen->OnInit != NULL) {
			currentScreen->OnInit();
			currentScreen->OnInit = NULL;
		}
		if (currentScreen->OnAwake != NULL)
			currentScreen->OnAwake();
	}
}
void ScreenManager_SetPrevious() {
	if (previousScreen == NULL)
		ScreenManager_SetScreen(&sTitleScreen);
	else
		ScreenManager_SetScreen(previousScreen);
}
void ScreenManager_Tick() {
	if (currentScreen->OnUpdate != NULL)
		currentScreen->OnUpdate();
}
void ScreenManager_DrawUp() {
	if (currentScreen->OnDrawUp != NULL)
		currentScreen->OnDrawUp();
}
void ScreenManager_DrawDown() {
	if (currentScreen->OnDrawDown != NULL)
		currentScreen->OnDrawDown();
}