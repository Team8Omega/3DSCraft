#include "client/gui/screens/PauseScreen.h"

#include "Globals.h"
#include "client/Game.h"
#include "client/gui/DebugUI.h"
#include "client/gui/Gui.h"
#include "client/gui/screens/SelectWorldScreen.h"
#include "client/renderer/SpriteBatch.h"

#include <citro3d.h>

static bool clicked_play = false;
static bool clicked_quit = false;

void PauseScreen_Draw();
void PauseScreen_Tick();

Screen sPauseScreen = { .DrawDown = PauseScreen_Draw, .Tick = PauseScreen_Tick };

void PauseScreen_Draw() {
	Gui_DrawBackgroundFull(3, -45);

	clicked_play = Gui_IconButton(0, 45, 160, 20, 0, true, SHADER_RGB(20, 20, 20), "Back to Game");
	clicked_quit = Gui_IconButton(0, 65, 160, 20, 0, true, SHADER_RGB(20, 20, 20), "Quit to Title");
}
void PauseScreen_Tick() {
	if (clicked_play) {
		Screen_SetScreen(SCREEN_NONE);
	}
	if (clicked_quit) {
		gReleaseWorld();
		Screen_SetScreen(SCREEN_SELECTWORLD);
	}
}
