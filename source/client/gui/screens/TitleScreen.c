#include <citro3d.h>
#include <client/gui/Gui.h>
#include <client/gui/screens/TitleScreen.h>
#include <client/gui/state_machine/state_machine.h>
#include <client/renderer/texture/SpriteBatch.h>

#include <client/model/VertexFmt.h>

extern void SelectWorldScreen(state_machine_t *machine);  // Forward declare state2

static bool clicked_play = false;
static bool clicked_quit = false;

void TitleScreen(state_machine_t *machine) {
	for (int i = 0; i < 160 / 16 + 1; i++) {
		for (int j = 0; j < 120 / 16 + 1; j++) {
			Gui_DrawBackground(0, i, j, -4);
		}
	}

	clicked_play = Gui_IconButton(0,32,240,21, "Singleplayer");
	clicked_quit = Gui_IconButton(0,54,240,21,"Quit");

	if (clicked_play) {
        clicked_play = false;
		state_machine_set_current_state(machine, SelectWorldScreen);
	}
	if (clicked_quit) {
		exit(0);
	}
}