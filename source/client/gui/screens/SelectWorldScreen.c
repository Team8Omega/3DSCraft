#include "client/gui/screens/SelectWorldScreen.h"

#include "client/Crash.h"
#include "client/Minecraft.h"
#include "client/gui/DebugUI.h"
#include "client/gui/Gui.h"
#include "client/gui/screens/ConfirmDeletionScreen.h"
#include "client/gui/screens/CreateWorldScreen.h"
#include "client/gui/screens/TitleScreen.h"
#include "client/renderer/CubeMap.h"
#include "client/renderer/SpriteBatch.h"
#include "util/Paths.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "client/player/Player.h"

#include <mpack/mpack.h>

#include <3ds.h>

WorldList worlds;

static void SelectWorldScreen_ScanWorlds() {
	vec_clear(&worlds.list);

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

			vec_push(&worlds.list, info);
		} else {
			DebugUI_Log("World cannot be fetched: \'%s\'", entry->d_name);
		}
	}

	closedir(directory);
}

void SelectWorldScreen_Init();
void SelectWorldScreen_Awake();
void SelectWorldScreen_Deinit();
void SelectWorldScreen_DrawDown();
void SelectWorldScreen_Tick();

Screen sSelectWorldScreen = { .OnInit	  = SelectWorldScreen_Init,
							  .OnAwake	  = SelectWorldScreen_Awake,
							  .OnDeinit	  = SelectWorldScreen_Deinit,
							  .OnDrawDown = SelectWorldScreen_DrawDown,
							  .OnUpdate	  = SelectWorldScreen_Tick };

void SelectWorldScreen_Init() {
	vec_init(&worlds.list);
}
void SelectWorldScreen_Awake() {
	SelectWorldScreen_ScanWorlds();
}
void SelectWorldScreen_Deinit() {
	vec_deinit(&worlds.list);
}

u16 worldNum = 0;

static int scroll		 = 0;
static float velocity	 = 0.f;
static u16 selectedWorld = 0;

static bool clicked_play		 = false;
static bool clicked_new_world	 = false;
static bool clicked_delete_world = false;
static bool clicked_back		 = false;

static float max_velocity = 20.f;

void SelectWorldScreen_DrawDown() {
	Gui_DrawBackgroundFull(0, -5);

	int boxOffset = 5;
	int boxWidth  = 60;
	int boxHeight = 75;
	int boxY	  = 17;
	int boxMargin = 10;	 // Variable for the margin between boxes

	int movementX = 0, movementY = 0;
	Gui_GetCursorMovement(&movementX, &movementY);
	if (Gui_IsCursorInside(0, 16, 160, (2 * 32) + 16)) {
		velocity += movementX / 2.f;
		velocity = CLAMP(velocity, -max_velocity, max_velocity);
	}
	scroll += velocity;
	velocity *= 0.75f;
	if (ABS(velocity) < 0.001f)
		velocity = 0.f;

	WorldInfo info;

	clicked_back = Gui_IconButton(0, 0, 15, 15, 0, true, SHADER_RGB(20, 20, 20), "<");

	Gui_Label_Centered(SpriteBatch_GetWidth(), 3, 0, 0, false, SHADER_RGB(4, 4, 4), "Singleplayer");

	SpriteBatch_PushSingleColorQuad(0, 0, -1, 320, 14, SHADER_RGB(25, 25, 25));

	vec_foreach (&worlds.list, info, worldNum) {
		int x;
		if (worlds.list.length == 1) {
			// Center the single box
			x = (SpriteBatch_GetWidth() - boxWidth) / 2 + scroll;
		} else {
			x = worldNum * (boxWidth + boxMargin) + boxOffset + boxMargin + scroll;	 // Adjusted x position calculation
		}

		Gui_DrawTint(x, boxY, boxWidth, boxHeight, -4, SHADER_RGB(6, 6, 6));
		Gui_DrawLine(x, 77, boxWidth, 1, -3, SHADER_RGB(4, 4, 4));
		Gui_DrawLine(x, 78, boxWidth, 1, -3, SHADER_RGB(7, 7, 7));
		Gui_DrawTint(x, boxY, boxWidth, 46, -2, SHADER_RGB(3, 3, 3));
		Gui_DrawLine(x, 90, boxWidth, 1, -3, SHADER_RGB(4, 4, 4));
		Gui_DrawLine(x, 91, boxWidth, 1, -3, SHADER_RGB(4, 4, 4));
		Gui_DrawOutline(x, boxY, boxWidth, boxHeight, 1, -2, SHADER_RGB(3, 3, 3));

		SpriteBatch_PushText(x + ((boxWidth / 2) - (SpriteBatch_CalcTextWidth(gamemodeTypesStr[gamemode]) / 2)), 54, 0, INT16_MAX, true,
							 INT_MAX, NULL, gamemodeTypesStr[gamemode], movementY);
		int truncateAt = 8;
		if (SpriteBatch_CalcTextWidth(info.name) >= 10) {
			SpriteBatch_PushText(x + 3, 66, 0, INT16_MAX, true, INT_MAX, NULL, "%s", SpriteBatch_TextTruncate(info.name, truncateAt),
								 movementY);
		} else {
			SpriteBatch_PushText(x + 3, 66, 0, INT16_MAX, true, INT_MAX, NULL, "%s", info.name, movementY);
		}

		// TODO: Delete button to link to Edit GUI; build Edit GUI
		if (Gui_EnteredCursorInside(x, 20, boxWidth, boxHeight)) {
			selectedWorld = worldNum;
		}
		if (selectedWorld == worldNum) {
			clicked_delete_world = Gui_IconButton(x, 77, boxWidth, 16, -1, true, SHADER_RGB(20, 20, 20), "Delete");
			clicked_play		 = Gui_IconButton(x, boxY, boxWidth, 60, -2, true, SHADER_RGB(20, 20, 20), "");
			clicked_play		 = Gui_Button(true, x + ((boxWidth / 2) - 10), 25, 20, 0, "|>");
			Gui_DrawOutline(x, boxY, boxWidth, boxHeight, 1, -1, SHADER_RGB(20, 20, 20));
		} else {
			SpriteBatch_PushText(x - 1 + ((boxWidth / 2) - (SpriteBatch_CalcTextWidth("Delete") / 2)), (77 + (CHAR_HEIGHT) / 2), -1,
								 INT16_MAX, false, INT_MAX, NULL, "Delete", movementY);
		}
	}

	if (worldNum == 0) {
		CreateWorldScreen(0, worldNum);
	}

	int maximumSize = ((worldNum * (boxWidth + boxMargin) + boxOffset + boxMargin) - SpriteBatch_GetWidth()) +
					  boxOffset;  // Adjusted maximum size calculation

	if (scroll < -maximumSize)
		scroll = -maximumSize;
	if (scroll > 0)
		scroll = 0;

	clicked_new_world = Gui_Button(true, 20, 97, 120, 0, "+   Create new world");
}

void SelectWorldScreen_Tick() {
	if (clicked_back) {
		clicked_back = false;
		GuiScreen_SetScreen(SCREEN_TITLE);
	}
	if (clicked_new_world) {
		clicked_new_world = false;
		CreateWorldScreen(selectedWorld, worldNum);
	}

	if (clicked_play && selectedWorld != -1) {
		clicked_play	= false;
		WorldInfo* info = &worlds.list.data[selectedWorld];
		gLoadWorld(info->path, info->name, 0, false);
	}
	if (clicked_delete_world && selectedWorld != -1) {
		clicked_delete_world = false;
		char buffer[512];
		sprintf(buffer, PATH_SAVES "%s", worlds.list.data[selectedWorld].path);
		ConfirmDeletionScreen(buffer);
	}
}