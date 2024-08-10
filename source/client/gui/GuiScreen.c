#include "client/gui/GuiScreen.h"

#include "client/gui/screens/ConfirmDeletionScreen.h"
#include "client/gui/screens/CreateWorldScreen.h"
#include "client/gui/screens/PauseScreen.h"
#include "client/gui/screens/SelectWorldScreen.h"
#include "client/gui/screens/TitleScreen.h"
//#include "client/gui/screens/.h"

Screen* currentScreen = NULL;

static Screen* screenList[SCREEN_COUNT] = {
	NULL, &sTitleScreen, &sPauseScreen, &sSelectWorldScreen, &sCreateWorldScreen, &sConfirmDeletionScreen,
};

void GuiScreen_SetScreen(u8 screen) {
	if (currentScreen != NULL && currentScreen->OnDeinit != NULL)
		currentScreen->OnDeinit();

	currentScreen = screenList[screen];

	if (currentScreen != NULL) {
		if (currentScreen->OnInit != NULL) {
			currentScreen->OnInit();
			currentScreen->OnInit = NULL;
		}
		if (currentScreen->OnAwake != NULL)
			currentScreen->OnAwake();
	}
}
void GuiScreen_Tick() {
	if (currentScreen->OnUpdate != NULL)
		currentScreen->OnUpdate();
}
void GuiScreen_DrawUp() {
	if (currentScreen->OnDrawUp != NULL)
		currentScreen->OnDrawUp();
}
void GuiScreen_DrawDown() {
	if (currentScreen->OnDrawDown != NULL)
		currentScreen->OnDrawDown();
}