#include "client/gui/DebugUI.h"

#include "client/gui/Gui.h"
#include "client/model/VertexFmt.h"
#include "client/renderer/texture/SpriteBatch.h"

#include "client/player/InputData.h"

#include "util/math/NumberUtils.h"

#include <stdarg.h>
#include <stdlib.h>

#ifdef _DEBUG
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

typedef enum
{
	MENUSTATE_NONE,
	MENUSTATE_LOG,
	MENUSTATE_MENUROOT,
	MENUSTATE_MENUTEST,
	MENUSTATE_COUNT
} DebugUI_State;

static bool isInfoBG			   = false;
static bool isShowButton		   = true;
static DebugUI_State menuState	   = MENUSTATE_NONE;
static DebugUI_State menuStateLast = MENUSTATE_NONE;
static u8 debugMenuOptionNum	   = 0;

static const DebugUI_Menu debugMenus[MENUSTATE_COUNT - 2] = {
	{ // Root Menu
	  	{
		  	{ 3, NULL, "sub" },	 // to sub menu
		  	{ 1, NULL, "log" }	 // to log
	  	},
	  	"Root"
	},
	{ // Sub Test Menu
	  	{
		  	{ 2, NULL, "root" },  // back to root
		  	{ 0, NULL, "close" }	  // close
	  	},
	  	"Sub Test Menu"
	}
};
#endif
void DebugUI_Init() {
#ifdef _DEBUG
	for (int i = 0; i < LOG_LINES; i++) {
		logLines[i] = malloc(LOG_LINE_LENGTH);
		memset(logLines[i], 0x0, LOG_LINE_LENGTH);
	}
	for (int i = 0; i < STATUS_LINES; i++) {
		statusLines[i] = malloc(STATUS_LINE_LENGTH);
		memset(statusLines[i], 0x0, STATUS_LINE_LENGTH);
	}
#endif
}
void DebugUI_Deinit() {
#ifdef _DEBUG
	for (int i = 0; i < LOG_LINES; i++)
		free(logLines[i]);
	for (int i = 0; i < STATUS_LINES; i++)
		free(statusLines[i]);
#endif
}

void DebugUI_Text(const char* text, ...) {
#ifdef _DEBUG
	if (currentStatusLine >= STATUS_LINES)
		return;
	va_list args;
	va_start(args, text);

	vsprintf(statusLines[currentStatusLine++], text, args);

	va_end(args);
#endif
}

void DebugUI_Log(const char* text, ...) {
#ifdef _DEBUG
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

#ifdef _DEBUG
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
}

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

	DebugUI_MenuSet(option->onPressMenuID);
}
void DebugUI_DrawMenu() {
	SpriteBatch_PushSingleColorQuad(0, 0, 99, 320, 240, SHADER_RGB(2, 2, 2));

	const DebugUI_Menu* menu = &debugMenus[menuState - 2];

	SpriteBatch_PushText(0, 0, 100, INT16_MAX, false, 320, 0, " 3DSCraft Debug Menu:   %s", menu->name);

	u8 yOffset =20;
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
#ifdef _DEBUG
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

	switch (menuState) {
		case MENUSTATE_LOG:
			DebugUI_DrawLog();
			break;
		case MENUSTATE_NONE:
			DebugUI_DrawInfo();
			break;
		default:
			DebugUI_DrawMenu();
			break;
	}
#endif
}
