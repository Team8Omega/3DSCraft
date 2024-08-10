#include "client/gui/screens/ConfirmDeletionScreen.h"

#include "client/Crash.h"
#include "client/gui/Gui.h"
#include "client/renderer/CubeMap.h"
#include "client/renderer/SpriteBatch.h"
#include "util/Paths.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char path[256];
static bool confirmed_deletion = false, canceled_deletion = false;

void ConfirmDeletionScreen_Draw();
void ConfirmDeletionScreen_Tick();

Screen sConfirmDeletionScreen = {
	.DrawDown = ConfirmDeletionScreen_Draw,
	.Tick	  = ConfirmDeletionScreen_Tick,
};

void ConfirmDeletionScreen(const char* path_) {
	strcpy(path, path_);
	Screen_SetScreen(SCREEN_CONFIRMDELETION);
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

void ConfirmDeletionScreen_Draw() {
	Gui_DrawBackgroundFull(0, -5);
	char* label = "Are you sure?";

	Gui_Label((SpriteBatch_GetWidth() / 2) - (SpriteBatch_CalcTextWidth(label) / 2), 20, 1, 0, true, INT16_MAX, label);

	canceled_deletion = Gui_Button(true, 5, 92, 70, 0, "No");

	confirmed_deletion = Gui_Button(true, 80, 92, 75, 0, "Yes");
}

void ConfirmDeletionScreen_Tick() {
	if (confirmed_deletion) {
		confirmed_deletion = false;
		delete_folder(path);
		Screen_SetScreen(SCREEN_SELECTWORLD);
	}
	if (canceled_deletion) {
		canceled_deletion = false;
		Screen_SetScreen(SCREEN_SELECTWORLD);
	}
}