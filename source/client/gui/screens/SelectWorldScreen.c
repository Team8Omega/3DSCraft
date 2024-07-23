
#include "client/gui/screens/SelectWorldScreen.h"

#include "client/Crash.h"
#include "client/gui/DebugUI.h"
#include "client/gui/Gui.h"
#include "client/gui/state_machine/state_machine.h"
#include "client/model/VertexFmt.h"
#include "client/renderer/texture/SpriteBatch.h"
#include "util/Paths.h"

#include <vec/vec.h>

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "client/player/Player.h"

#include <mpack/mpack.h>

#include <3ds.h>

typedef struct {
	uint32_t lastPlayed;
	char name[WORLD_NAME_SIZE];
	char path[256];
} WorldInfo;

static vec_t(WorldInfo) worlds;

// Declare machine as a global variable
static state_machine_t* machine;

void SelectWorldScreen_ScanWorlds() {
	vec_clear(&worlds);

	DIR* directory = opendir(PATH_SAVES);

	char buffer[512];

	struct dirent* entry;

	while ((entry = readdir(directory))) {
		sprintf(buffer, PATH_SAVES "%s/level.mp", entry->d_name);
		if (access(buffer, F_OK) != -1) {
			mpack_tree_t tree;
			mpack_tree_init_file(&tree, buffer, 0);
			mpack_node_t root = mpack_tree_root(&tree);

			char name[WORLD_NAME_SIZE];
			mpack_node_copy_utf8_cstr(mpack_node_map_cstr(root, "name"), name, WORLD_NAME_SIZE);

			u8 err = mpack_tree_destroy(&tree);
			if (err != mpack_ok) {
				Crash("Mpack failure \'%d\' while loading world %s", err, entry->d_name);
				continue;
			}

			WorldInfo info;
			strcpy(info.name, name);
			info.lastPlayed = 0;
			strcpy(info.path, entry->d_name);

			vec_push(&worlds, info);
		}
	}

	closedir(directory);
}

static void delete_folder(const char* path) {
	DIR* dir = opendir(path);
	struct dirent* entry;

	int pathLen = strlen(path);

	while ((entry = readdir(dir))) {
		if (!strcmp(entry->d_name, "..") || !strcmp(entry->d_name, "."))
			continue;

		int entryLen = strlen(entry->d_name);

		char buffer[pathLen + entryLen + 1];
		sprintf(buffer, "%s/%s", path, entry->d_name);

		if (entry->d_type == DT_DIR)
			delete_folder(buffer);
		else
			unlink(buffer);
	}

	closedir(dir);

	rmdir(path);
}

void SelectWorldScreen_Init(state_machine_t* sm) {
	vec_init(&worlds);
	machine = sm;  // Assign the state machine pointer

	SelectWorldScreen_ScanWorlds();
}

void SelectWorldScreen_Deinit() {
	vec_deinit(&worlds);
}

// TODO: Separate menu states
typedef enum
{
	MenuState_SelectWorld,
	MenuState_ConfirmDeletion,
	MenuState_WorldOptions
} MenuState;

static int scroll		 = 0;
static float velocity	 = 0.f;
static int selectedWorld = -1;

static bool clicked_play		 = false;
static bool clicked_new_world	 = false;
static bool clicked_delete_world = false;
static bool clicked_back		 = false;

static bool confirmed_world_options = false;
static bool canceled_world_options	= false;

static bool confirmed_deletion = false;
static bool canceled_deletion  = false;

static MenuState menustate = MenuState_SelectWorld;

static WorldGenType worldGenType = WorldGen_SuperFlat;

static Gamemode gamemode = Gamemode_Survival;

static Difficulty difficulty = Difficulty_Normal;

static char* worldGenTypesStr[] = { "Default", "Superflat" };

static char* gamemodeTypesStr[] = { "Survival", "Creative", "Adventure", "Spectator" };

static char* difficultyTypesStr[] = { "Normal", "Peaceful", "Easy", "Hard", "Hardcore" };

static float max_velocity = 20.f;

bool overlay = false;

extern void TitleScreen(state_machine_t* machine);	// Forward declare state1

void SelectWorldScreen(state_machine_t* sm) {
	machine = sm;  // Update machine when state2 is called with a new state_machine_t

	for (int i = 0; i < 160 / 16 + 1; i++) {
		for (int j = 0; j < 120 / 16 + 1; j++) {
			bool overlay = j >= 2 && menustate == MenuState_SelectWorld;
			Gui_DrawBackground(overlay ? 0 : 1, i, j, overlay ? -4 : -10);
		}
	}
	if (menustate == MenuState_SelectWorld) {
		int movementX = 0, movementY = 0;
		Gui_GetCursorMovement(&movementX, &movementY);
		if (Gui_IsCursorInside(0, 0, 160, 2 * 32)) {
			velocity += movementY / 2.f;
			velocity = CLAMP(velocity, -max_velocity, max_velocity);
		}
		scroll += velocity;
		velocity *= 0.75f;
		if (ABS(velocity) < 0.001f)
			velocity = 0.f;

		int maximumSize = CHAR_HEIGHT * 2 * worlds.length;
		if (scroll < -maximumSize)
			scroll = -maximumSize;
		if (scroll > 0)
			scroll = 0;

		WorldInfo info;
		int i = 0;
		vec_foreach (&worlds, info, i) {
			int y = i * (CHAR_HEIGHT + CHAR_HEIGHT) + 10 + scroll;
			if (Gui_EnteredCursorInside(10, y - 3, 140, CHAR_HEIGHT + 6) && y < 32 * 2) {
				selectedWorld = i;
			}
			if (selectedWorld == i) {
				SpriteBatch_PushSingleColorQuad(10, y - 3, -7, 140, 1, SHADER_RGB(20, 20, 20));
				SpriteBatch_PushSingleColorQuad(10, y + CHAR_HEIGHT + 2, -7, 140, 1, SHADER_RGB(20, 20, 20));
				SpriteBatch_PushSingleColorQuad(10, y - 3, -7, 1, CHAR_HEIGHT + 6, SHADER_RGB(20, 20, 20));
				SpriteBatch_PushSingleColorQuad(10 + 140, y - 3, -7, 1, CHAR_HEIGHT + 6, SHADER_RGB(20, 20, 20));
			}
			SpriteBatch_PushText(20, y, -6, INT16_MAX, true, INT_MAX, NULL, "%s", info.name, movementY);
		}

		int margin = 5;
		DebugUI_Text("%d", selectedWorld);
		clicked_play = Gui_Button(selectedWorld != -1, margin, 70, 155 - margin, 0, "Play selected world");

		clicked_new_world	 = Gui_Button(true, 5, 92, 50, 0, "New");
		clicked_delete_world = Gui_Button(selectedWorld != -1, 57, 92, 50, 0, "Delete");
		clicked_back		 = Gui_Button(true, 110, 92, 50 - 5, 0, "Back");

	} else if (menustate == MenuState_ConfirmDeletion) {
		char* label = "Are you sure?";

		Gui_Label((SpriteBatch_GetWidth() / 2) - (SpriteBatch_CalcTextWidth(label) / 2), 20, 1, 0, true, INT16_MAX, label);

		canceled_deletion = Gui_Button(true, 5, 92, 70, 0, "No");

		confirmed_deletion = Gui_Button(true, 80, 92, 75, 0, "Yes");

	} else if (menustate == MenuState_WorldOptions) {
		Gui_Label(5, 15, 0, 0, true, INT16_MAX, "World type:");
		if (Gui_Button(true, 95, 10, 60, 0, worldGenTypesStr[worldGenType])) {
			worldGenType++;
			if (worldGenType == WorldGenTypes_Count)
				worldGenType = 0;
		}

		Gui_Label(5, 40, 0, 0, true, INT16_MAX, "Game Mode:");
		if (Gui_Button(true, 95, 35, 60, 0, gamemodeTypesStr[gamemode])) {
			gamemode++;

			// player->gamemode = gamemode;
			if (gamemode == Gamemode_Count)
				gamemode = 0;
		}

		Gui_Label(5, 65, 0, 0, true, INT16_MAX, "Difficulty:");
		if (Gui_Button(true, 95, 60, 60, 0, difficultyTypesStr[difficulty])) {
			difficulty++;

			// player->difficulty = difficulty;
			if (difficulty == Difficulty_Count)
				difficulty = 0;
		}

		canceled_world_options = Gui_Button(true, 5, 92, 70, 0, "Cancel");

		confirmed_world_options = Gui_Button(true, 80, 92, 75, 0, "Continue");
	}
}

bool SelectWorldScreen_Update(char* out_worldpath, char* out_name, WorldGenType* worldType, bool* newWorld) {
	if (clicked_back) {
		clicked_back = false;
		state_machine_set_current_state(machine, TitleScreen);
	}
	if (clicked_new_world) {
		clicked_new_world = false;
		menustate		  = MenuState_WorldOptions;
	}
	if (confirmed_world_options) {
		confirmed_world_options = false;
		*worldType				= worldGenType;
		// player->gamemode=gamemode3;

		static SwkbdState swkbd;
		static char name[WORLD_NAME_SIZE];

		// #ifndef _DEBUG
		swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, WORLD_NAME_SIZE);
		swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, WORLD_NAME_SIZE);
		swkbdSetHintText(&swkbd, "Enter the world name");

		int button = swkbdInputText(&swkbd, name, 12);
		/*#else
				strcpy(name, "testworld");
				int button = 2;
		#endif*/

		strcpy(out_name, name);
		menustate = MenuState_SelectWorld;
		if (button == SWKBD_BUTTON_CONFIRM) {
			strcpy(out_worldpath, out_name);

			int length = strlen(out_worldpath);

			for (int i = 0; i < length; i++) {
				if (out_worldpath[i] == '/' || out_worldpath[i] == '\\' || out_worldpath[i] == '?' || out_worldpath[i] == ':' ||
					out_worldpath[i] == '|' || out_worldpath[i] == '<' || out_worldpath[i] == '>')
					out_worldpath[i] = '_';
			}

			while (true) {
				int i;
				WorldInfo* info;
				bool alreadyExisting = false;
				vec_foreach_ptr(&worlds, info, i) if (!strcmp(out_worldpath, info->path)) {
					alreadyExisting = true;
					break;
				}
				if (!alreadyExisting)
					break;

				out_worldpath[length]	  = '_';
				out_worldpath[length + 1] = '\0';
				++length;
			}

			*newWorld = true;

			return true;
		}
	}
	if (clicked_play && selectedWorld != -1) {
		clicked_play = false;
		strcpy(out_name, worlds.data[selectedWorld].name);
		strcpy(out_worldpath, worlds.data[selectedWorld].path);

		*newWorld = false;
		menustate = MenuState_SelectWorld;
		return true;
	}
	if (clicked_delete_world && selectedWorld != -1) {
		clicked_delete_world = false;
		menustate			 = MenuState_ConfirmDeletion;
	}
	if (confirmed_deletion) {
		confirmed_deletion = false;
		char buffer[512];
		sprintf(buffer, PATH_SAVES "%s", worlds.data[selectedWorld].path);
		delete_folder(buffer);

		SelectWorldScreen_ScanWorlds();
		menustate = MenuState_SelectWorld;
	}
	if (canceled_deletion) {
		canceled_deletion = false;
		menustate		  = MenuState_SelectWorld;
	}
	if (canceled_world_options) {
		canceled_world_options = false;
		menustate			   = MenuState_SelectWorld;
	}

	return false;
}