#include "client/gui/screens/TitleScreen.h"

#include "client/Camera.h"
#include "client/Crash.h"
#include "client/Game.h"
#include "client/gui/Gui.h"
#include "client/gui/screens/SelectWorldScreen.h"
#include "client/model/VertexFmt.h"
#include "client/renderer/CubeMap.h"
#include "client/renderer/Shader.h"
#include "client/renderer/SpriteBatch.h"
#include "client/renderer/texture/TextureMap.h"

#include "Globals.h"

static bool clicked_play = false;
static bool clicked_quit = false;
C3D_Tex texLogo;

void TitleScreen_Init();
void TitleScreen_Deinit();
void TitleScreen_DrawDown();
void TitleScreen_DrawUp();

Screen sTitleScreen = { .Init = TitleScreen_Init, .DrawDown = TitleScreen_DrawDown };

extern Shader shaderWorld;

void TitleScreen_Init() {
	if (texLogo.data != NULL)
		return;

	Texture_Load(&texLogo, "gui/title/3dscraft.png");

	CubeMap_Init(shaderWorld.uLocProjection);

	CubeMap_Set("gui/title/background/panorama", f3_new(0.f, 0.f, 0.f));
}
void TitleScreen_Deinit() {
	C3D_TexDelete(&texLogo);

	CubeMap_Deinit();
}
void TitleScreen_DrawDown() {
	Gui_DrawBackgroundFull(0, -4);

	clicked_play = Gui_IconButton(0, 45, 160, 20, 0, true, SHADER_RGB(20, 20, 20), localeGet[LOC_MENU_SINGLEPLAYER]);
	clicked_quit = Gui_IconButton(0, 65, 160, 20, 0, true, SHADER_RGB(20, 20, 20), localeGet[LOC_MENU_QUIT]);

	if (clicked_play) {
		clicked_play = false;
		Screen_SetScreen(SCREEN_SELECTWORLD);
	}
	if (clicked_quit) {
		gStop();
	}
}

void TitleScreen_DrawUp() {
	SpriteBatch_SetScale(2);

	SpriteBatch_BindTexture(&texLogo);

	SpriteBatch_PushQuad(28, 20, 0, 144, 36, 0, 0, 512, 128);

	SpriteBatch_PushText(0, 112, 0, INT16_MAX, false, INT_MAX, NULL, APP_VERSION);

	SpriteBatch_PushText(210 - 70, 112, 0, INT16_MAX, false, INT_MAX, NULL, AUTHOR);

	SpriteBatch_SetScale(1);
}