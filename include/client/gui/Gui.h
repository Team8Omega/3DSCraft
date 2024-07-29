#pragma once

#include <stdbool.h>
#include <stdint.h>

void Gui_Init();

void Gui_Deinit();

void Gui_Label(int x, int y, int z, float size, bool shadow, int16_t color, const char* text, ...);
// this element is centered around the X axis
void Gui_Label_Centered(int x, int y, int z, float size, bool shadow, int16_t color, const char* text, ...);
#define BUTTON_HEIGHT 20
#define BUTTON_TEXT_PADDING ((BUTTON_HEIGHT - CHAR_HEIGHT) / 2)
bool Gui_Button(bool enabled, int x, int y, int width, int z, const char* label);

bool Gui_IconButton(int x, int y, int width, int height, int z, bool centered, uint32_t tintColor, const char* label);

bool Gui_IsCursorInside(int x, int y, int w, int h);

bool Gui_WasCursorInside(int x, int y, int w, int h);

void Gui_GetCursorMovement(int* x, int* y);

bool Gui_EnteredCursorInside(int x, int y, int w, int h);

void Gui_DrawLine(int x, int y, int width, int thickness, int z, uint32_t color);

void Gui_DrawOutline(int x, int y, int width, int height, int thickness, int z, uint32_t color);

void Gui_DrawTint(int x, int y, int width, int height, int z, uint32_t color);

void Gui_DrawBackgroundFull(u8 index, s8 depth);

void Gui_DrawBackground(u8 background, int x, int y, int z);
