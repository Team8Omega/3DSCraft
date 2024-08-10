#include "client/gui/screens/CreateWorldScreen.h"

#include "Globals.h"
#include "client/Crash.h"
#include "client/Minecraft.h"
#include "client/gui/Gui.h"
#include "client/gui/screens/SelectWorldScreen.h"
#include "client/gui/screens/TitleScreen.h"
#include "client/renderer/CubeMap.h"

WorldGenType worldGenType		 = WorldGen_SuperFlat;
Gamemode gamemode				 = Gamemode_Survival;
Difficulty difficulty			 = Difficulty_Normal;
const char* worldGenTypesStr[]	 = { "Default", "Superflat" };
const char* gamemodeTypesStr[]	 = { "Survival", "Hardcore", "Creative", "Spectator" };
const char* difficultyTypesStr[] = {
	"Normal",
	"Hard",
	"Peaceful",
	"Easy",
};

static u16 selectedWorld = 0, worldNum = 0;

static bool confirmed_world_options = false;
static bool canceled_world_options	= false;

void CreateWorldScreen_Draw();
void CreateWorldScreen_Tick();

Screen sCreateWorldScreen = {
	.Tick	  = CreateWorldScreen_Tick,
	.DrawDown = CreateWorldScreen_Draw,
};

void CreateWorldScreen(u16 selectedIdx, u16 worldNo) {
	selectedWorld = selectedIdx;
	worldNum	  = worldNo;
	Screen_SetScreen(SCREEN_CREATEWORLD);
}

void CreateWorldScreen_Draw() {
	Gui_DrawBackgroundFull(0, -5);
	Gui_Label(5, 15, 0, 0, true, INT16_MAX, "World type:");
	if (Gui_Button(true, 95, 10, 60, 0, worldGenTypesStr[worldGenType])) {
		worldGenType++;
		if (worldGenType == WorldGenTypes_Count)
			worldGenType = 0;
	}

	Gui_Label(5, 40, 0, 0, true, INT16_MAX, "Game Mode:");
	if (Gui_Button(true, 95, 35, 60, 0, gamemodeTypesStr[gamemode])) {
		gamemode++;

		// Spectator mode doesnt need to be rendered
		if (gamemode == Gamemode_Spectator) {
			gamemode++;
		}
		// gPlayer->gamemode = gamemode;
		if (gamemode >= Gamemode_Count)
			gamemode = 0;
	}

	Gui_Label(5, 65, 0, 0, true, INT16_MAX, "Difficulty:");
	if (Gui_Button(true, 95, 60, 60, 0, difficultyTypesStr[difficulty])) {
		difficulty++;

		// gPlayer->difficulty = difficulty;
		if (difficulty == Difficulty_Count)
			difficulty = 0;
	}

	canceled_world_options = Gui_Button(true, 5, 92, 70, 0, "Cancel");

	confirmed_world_options = Gui_Button(true, 80, 92, 75, 0, "Continue");
}
void CreateWorldScreen_Tick() {
	if (confirmed_world_options) {
		confirmed_world_options = false;
		// gPlayer->gamemode=gamemode3;

		static char name[WORLD_NAME_SIZE];

#ifndef DEBUG_WORLDNAME
		static SwkbdState swkbd;
		swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, WORLD_NAME_SIZE);
		swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, WORLD_NAME_SIZE);
		swkbdSetHintText(&swkbd, "Enter the world name");

		int button = swkbdInputText(&swkbd, name, 256);
#else
		strcpy(name, "testworld");
		int button = 2;
#endif
		char worldpath[256];
		if (button == SWKBD_BUTTON_CONFIRM) {
			strcpy(worldpath, name);

			int length = strlen(worldpath);

			for (int i = 0; i < length; i++) {
				if (worldpath[i] == '/' || worldpath[i] == '\\' || worldpath[i] == '?' || worldpath[i] == ':' || worldpath[i] == '|' ||
					worldpath[i] == '<' || worldpath[i] == '>')
					worldpath[i] = '_';
			}

			while (true) {
				int i;
				WorldInfo* info;
				bool alreadyExisting = false;
				vec_foreach_ptr(&worlds.list, info, i) if (!strcmp(worldpath, info->path)) {
					alreadyExisting = true;
					break;
				}
				if (!alreadyExisting)
					break;

				worldpath[length]	  = '_';
				worldpath[length + 1] = '\0';
				++length;
			}
			gLoadWorld(name, name, worldGenType, true);
		}
	}
	if (canceled_world_options) {
		canceled_world_options = false;
		if (worldNum == 0) {
			Screen_SetScreen(SCREEN_TITLE);
		} else {
			Screen_SetScreen(SCREEN_SELECTWORLD);
		}
	}
}