#include "client/gui/DebugUI.h"

#include "client/gui/Gui.h"
#include "client/model/VertexFmt.h"
#include "client/renderer/SpriteBatch.h"

#include "client/player/InputData.h"

#include "Globals.h"
#include "util/math/NumberUtils.h"

#include "client/Crash.h"
#include "util/Paths.h"

#include <dirent.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef DEBUG_UI
#define STATUS_LINES (240 / 8 / 2)
#define LOG_LINES 30
#define LOG_LINE_LENGTH 128
#define STATUS_LINE_LENGTH 128

static char* statusLines[STATUS_LINES];
static char* logLines[LOG_LINES];
static int currentStatusLine = 0;

typedef void (*Function)();

typedef struct {
	const u8 onPressMenuID;
	const Function onPressFunc;
	const char* label;
} DebugUI_MenuOption;

typedef struct {
	const DebugUI_MenuOption options[64];
	const char* name;
} DebugUI_Menu;

typedef u8 DebugUI_State;
enum
{
	MENUSTATE_NONE,
	MENUSTATE_LOG,
	MENUSTATE_MENUROOT,
	MENUSTATE_MENUDBGCOMMON,
	MENUSTATE_MENUDBGINGAME,
	MENUSTATE_COUNT
};

static bool isLogPaused			   = false;
static bool isInfoBG			   = false;
static bool isShowButton		   = true;
static DebugUI_State menuState	   = MENUSTATE_NONE;
static DebugUI_State menuStateLast = MENUSTATE_NONE;
static u8 debugMenuOptionNum	   = 0;

static const DebugUI_Menu debugMenus[MENUSTATE_COUNT - 2] = { { {
																	{ 3, NULL, "Debug Common" },  // to sub menu
																	{ 4, NULL, "Debug Ingame" }	  // to log
																},
																"Root" },
															  { {
																	{ 0xFF, NULL, "(didnt work)" }	// delete sdmc
																},
																"Debug Common" },
															  { {
																	{ 2, NULL, "root" },  // back to root
																	{ 0, NULL, "close" }  // close
																},
																"Debug Ingame" } };
#endif
void DebugUI_Init() {
#ifdef DEBUG_LOG
	for (int i = 0; i < LOG_LINES; i++) {
		logLines[i] = malloc(LOG_LINE_LENGTH);
		memset(logLines[i], 0x0, LOG_LINE_LENGTH);
	}
#endif
#ifdef DEBUG_INFO
	for (int i = 0; i < STATUS_LINES; i++) {
		statusLines[i] = malloc(STATUS_LINE_LENGTH);
		memset(statusLines[i], 0x0, STATUS_LINE_LENGTH);
	}
#endif
}
void DebugUI_Deinit() {
#ifdef DEBUG_LOG
	for (int i = 0; i < LOG_LINES; i++)
		free(logLines[i]);
#endif
#ifdef DEBUG_INFO
	for (int i = 0; i < STATUS_LINES; i++)
		free(statusLines[i]);
#endif
}

void DebugUI_Text(const char* text, ...) {
#ifdef DEBUG_INFO
	if (currentStatusLine >= STATUS_LINES)
		return;
	va_list args;
	va_start(args, text);

	vsprintf(statusLines[currentStatusLine++], text, args);

	va_end(args);
#endif
}

void DebugUI_Log(const char* text, ...) {
#ifdef DEBUG_LOG
	if (isLogPaused)
		return;

	if (strlen(text) < 0)
		return;

	va_list args;
	va_start(args, text);

	char stringFormatted[512];
	vsprintf(stringFormatted, text, args);

	if (strlen(stringFormatted) < 0)
		return;

	va_end(args);

	char* lastLine = logLines[LOG_LINES - 1];
	for (int i = LOG_LINES - 1; i > 0; i--)
		logLines[i] = logLines[i - 1];
	logLines[0] = lastLine;

	strncpy(logLines[0], stringFormatted, LOG_LINE_LENGTH);
	logLines[0][LOG_LINE_LENGTH - 1] = '\0';

	va_end(args);
#endif
}

#ifdef DEBUG_INFO
void DebugUI_DrawInfo() {
	u8 infoNum;
	u8 yOffset = 0;
	for (infoNum = 0; infoNum < STATUS_LINES; infoNum++) {
		if (strcmp(statusLines[infoNum], "") == 0)
			break;

		int step = 0;
		SpriteBatch_PushText(0, yOffset, 98, INT16_MAX, false, 320, &step, "%s", statusLines[infoNum]);
		yOffset += step;

		memset(statusLines[infoNum], 0x0, STATUS_LINE_LENGTH);
	}
	currentStatusLine = 0;

	if (isInfoBG)
		SpriteBatch_PushSingleColorQuad(0, 0, 97, 320, (8 * (infoNum + 1)) + 4, SHADER_RGB(2, 2, 2));
}
#endif
#ifdef DEBUG_LOG
void DebugUI_DrawLog() {
	u8 yOffset = 12;
	SpriteBatch_PushSingleColorQuad(0, 0, 99, 320, 240, SHADER_RGB(2, 2, 2));

	SpriteBatch_PushText(0, 0, 100, INT16_MAX, false, 320, 0, " Debug Log");
	for (int i = 0; i < LOG_LINES; i++) {
		if (strlen(logLines[i]) == 0)
			continue;
		int step = 0;
		SpriteBatch_PushText(1, yOffset, 100, INT16_MAX, false, 320, &step, "[LOG] %s", logLines[i]);
		yOffset += step;
		if (yOffset >= 240)
			break;
	}
	if (Gui_Button(true, 320 - 45, 54, 40, 100, "Pause"))
		isLogPaused = isLogPaused ? false : true;
}
#endif
#ifdef DEBUG_UI
void DebugUI_MenuSet(DebugUI_State menuid) {
	menuStateLast = menuState;
	menuState	  = menuid;

	if (menuid > 1) {
		const DebugUI_Menu* menu = &debugMenus[menuState - 2];

		int sizeOpt = sizeof(menu->options);

		for (debugMenuOptionNum = 0; debugMenuOptionNum < sizeOpt; ++debugMenuOptionNum) {
			if (menu->options[debugMenuOptionNum].label == NULL)
				break;
		}
	}
}
void DebugUI_MenuCall(const DebugUI_MenuOption* option) {
	if (option->onPressFunc)
		option->onPressFunc();

	if (option->onPressMenuID != 0xFF)
		DebugUI_MenuSet(option->onPressMenuID);
}
void DebugUI_DrawMenu() {
	SpriteBatch_PushSingleColorQuad(0, 0, 99, 320, 240, SHADER_RGB(2, 2, 2));

	const DebugUI_Menu* menu = &debugMenus[menuState - 2];

	SpriteBatch_PushText(0, 0, 100, INT16_MAX, false, 320, 0, " 3DSCraft Debug Menu:   %s", menu->name);

	u8 yOffset = 20;
	for (u8 i = 0; i < debugMenuOptionNum; ++i) {
		const DebugUI_MenuOption* option = &menu->options[i];

		if (Gui_Button(true, 10, yOffset, 100, 100, option->label))
			DebugUI_MenuCall(option);

#define OFFSET_Y_MAX 220
		yOffset += 20;
		if (yOffset > OFFSET_Y_MAX)
			break;	// todo: scroll
	}
	if (Gui_Button(true, 10, OFFSET_Y_MAX, 100, 100, "Back"))
		DebugUI_MenuSet(menuStateLast);
}
#endif

void DebugUI_Draw() {
#ifdef DEBUG_UI
	SpriteBatch_SetScale(1);

	if (gInput.keysdown)
		if (gInput.keysheld & KEY_L && gInput.keysheld & KEY_SELECT && gInput.keysdown & KEY_UP)
			isShowButton = isShowButton ? false : true;

	if (isShowButton) {
		if (Gui_Button(true, 320 - 32, 0, 32, 100, "LOG"))
			menuState == MENUSTATE_LOG ? DebugUI_MenuSet(MENUSTATE_NONE) : DebugUI_MenuSet(MENUSTATE_LOG);
		if (Gui_Button(true, 320 - 32, 20, 32, 100, "DBG"))
			menuState > 1 ? DebugUI_MenuSet(MENUSTATE_NONE) : DebugUI_MenuSet(MENUSTATE_MENUROOT);
		if (Gui_Button(true, 320 - 32, 38, 32, 100, "INFO"))
			isInfoBG = isInfoBG ? false : true;
	}
#endif

#ifdef _DEBUG
	switch (menuState) {
#ifdef DEBUG_LOG
		case MENUSTATE_LOG:
			DebugUI_DrawLog();
			break;
#endif
#ifdef DEBUG_INFO
		case MENUSTATE_NONE:
			DebugUI_DrawInfo();
			break;
#endif
#ifdef DEBUG_UI
		default:
			DebugUI_DrawMenu();
			break;
#endif
	}
#endif
}
