#pragma once

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

#include "client/player/InputData.h"
#include "world/World.h"

#include <citro3d.h>

void SpriteBatch_Init(int projUniform_);
void SpriteBatch_Deinit();

typedef u8 GuiTexture;
enum {
	GuiTexture_Blank,
	GuiTexture_Font,
	GuiTexture_Icons,
	GuiTexture_Widgets,
	GuiTexture_MenuBackground
};

void SpriteBatch_BindGuiTexture(GuiTexture texture);
void SpriteBatch_BindTexture(C3D_Tex* texture);

void SpriteBatch_PushSingleColorQuad(s16 x, s16 y, s16 z, s16 w, s16 h, s16 color);
void SpriteBatch_PushQuadColor(s16 x, s16 y, s16 z, s16 w, s16 h, s16 rx, s16 ry, s16 rw, s16 rh, s16 color);
void SpriteBatch_PushQuad(s16 x, s16 y, s16 z, s16 w, s16 h, s16 rx, s16 ry, s16 rw, s16 rh);
void SpriteBatch_PushIcon(BlockId block, u8 metadata, s16 x, s16 y, s16 z);

#define CHAR_HEIGHT 8
int SpriteBatch_PushTextVargs(int x, int y, int z, s16 color, bool shadow, int wrap, int* ySize, const char* fmt, va_list arg);
int SpriteBatch_PushText(int x, int y, int z, s16 color, bool shadow, int wrap, int* ySize, const char* fmt, ...);

int SpriteBatch_CalcTextWidthVargs(const char* text, va_list args);
int SpriteBatch_CalcTextWidth(const char* text, ...);

char* SpriteBatch_TextTruncate(const char* text, size_t length);

bool SpriteBatch_RectIsVisible(int x, int y, int w, int h);

void SpriteBatch_SetScale(int scale);
int SpriteBatch_GetScale();

int SpriteBatch_GetWidth();
int SpriteBatch_GetHeight();

void SpriteBatch_Render();

void SpriteBatch_SetScreen(bool isTop);