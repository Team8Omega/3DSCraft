#include "client/gui/screens/TitleScreen.h"

#include "client/Camera.h"
#include "client/Crash.h"
#include "client/gui/Gui.h"
#include "client/gui/screens/SelectWorldScreen.h"
#include "client/model/VertexFmt.h"
#include "client/renderer/CubeMap.h"
#include "client/renderer/Shader.h"
#include "client/renderer/texture/SpriteBatch.h"
#include "client/renderer/texture/TextureMap.h"

#include "Globals.h"

static bool clicked_play = false;
static bool clicked_quit = false;
C3D_Tex texLogo;

void TitleScreen_Init();
void TitleScreen_Deinit();
void TitleScreen_DrawDown();
void TitleScreen_DrawUp();

Screen sTitleScreen = { .OnDrawDown = TitleScreen_DrawDown };

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
	for (int i = 0; i < 160 / 16 + 1; i++) {
		for (int j = 0; j < 120 / 16 + 1; j++) {
			Gui_DrawBackground(0, i, j, -4);
		}
	}

	clicked_play = Gui_IconButton(0, 45, 160, 20, 0, true, SHADER_RGB(20, 20, 20), "Singleplayer");
	clicked_quit = Gui_IconButton(0, 65, 160, 20, 0, true, SHADER_RGB(20, 20, 20), "Quit");

	if (clicked_play) {
		clicked_play = false;
		ScreenManager_SetScreen(&sSelectWorldScreen);
	}
	if (clicked_quit) {
		exit(0);
	}
}

void TitleScreen_DrawUp() {
	CubeMap_Update(&gCamera.projection, f3_new(0.f, 0.0013f, 0.f));
	CubeMap_Draw();

	SpriteBatch_SetScale(2);

	SpriteBatch_BindTexture(&texLogo);

	SpriteBatch_PushQuad(30, 30, 0, 140, 35, 0, 0, 512, 256);

	SpriteBatch_PushText(0, 112, 0, INT16_MAX, false, INT_MAX, NULL, APP_VERSION);

	SpriteBatch_PushText(210 - 70, 112, 0, INT16_MAX, false, INT_MAX, NULL, AUTHOR);

	SpriteBatch_SetScale(1);
}