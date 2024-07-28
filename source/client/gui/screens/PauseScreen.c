#include <citro3d.h>
#include <client/gui/Gui.h>
#include <client/gui/screens/TitleScreen.h>
#include <client/gui/state_machine/state_machine.h>
#include <client/renderer/texture/SpriteBatch.h>
#include "client/gui/DebugUI.h"
#include <client/model/VertexFmt.h>
#include <Globals.h>

static bool clicked_play = false;
static bool clicked_quit = false;

void PauseScreen(GameState** gamestate) {
	for (int i = 0; i < 160 / 16 + 1; i++) {
		for (int j = 0; j < 120 / 16 + 1; j++) {
			Gui_DrawBackground(3, i, j, -4);
		}
	}

	clicked_play = Gui_IconButton(0, 45, 160, 20, 0, true, SHADER_RGB(20, 20, 20), "Back to Game");
	clicked_quit = Gui_IconButton(0, 65, 160, 20, 0, true, SHADER_RGB(20, 20, 20), "Quit Game");

	if (clicked_play) {
		**gamestate = GameState_Playing;
		DebugUI_Log("Clicked Play, However GameState pointer doesn't change");
	}
	if (clicked_quit) {
		**gamestate = GameState_Menu;
		DebugUI_Log("Clicked Quit, However GameState pointer doesn't change");
	}
}
