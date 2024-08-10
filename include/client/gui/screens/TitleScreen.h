#pragma once

#include "client/gui/GuiScreen.h"
#include <citro3d.h>

extern Screen sTitleScreen;

extern C3D_Tex texLogo;

void TitleScreen_Init();

void TitleScreen_Deinit();

void TitleScreen_DrawUp();
