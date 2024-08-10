#include "client/gui/Screen.h"

#include "client/gui/screens/ConfirmDeletionScreen.h"
#include "client/gui/screens/CreateWorldScreen.h"
#include "client/gui/screens/PauseScreen.h"
#include "client/gui/screens/SelectWorldScreen.h"
#include "client/gui/screens/TitleScreen.h"
//#include "client/gui/screens/.h"

Screen* currentScreen		 = NULL;
ScreenIndex currentScreenIdx = 0;

static Screen* screenList[SCREEN_COUNT] = {
	NULL, &sTitleScreen, &sPauseScreen, &sSelectWorldScreen, &sCreateWorldScreen, &sConfirmDeletionScreen,
};

void Screen_SetScreen(ScreenIndex screen) {
	if (currentScreen != NULL && currentScreen->Deinit != NULL)
		currentScreen->Deinit();

	currentScreen	 = screenList[screen];
	currentScreenIdx = screen;

	if (currentScreen != NULL) {
		if (currentScreen->Init != NULL) {
			currentScreen->Init();
			currentScreen->Init = NULL;
		}
		if (currentScreen->Awake != NULL)
			currentScreen->Awake();
	}
}
void Screen_Tick() {
	if (currentScreen->Tick)
		currentScreen->Tick();
}
void Screen_DrawUp() {
	if (currentScreen->DrawUp)
		currentScreen->DrawUp();
}
void Screen_DrawDown() {
	if (currentScreen->DrawDown)
		currentScreen->DrawDown();
}