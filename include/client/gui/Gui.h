#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "client/player/InputData.h"

void Gui_Init();
void Gui_Deinit();

void Gui_InputData(InputData data);

void Gui_Label(int x, int y, int z, float size, bool shadow, int16_t color, const char* text, ...);
//this element is centered around the X axis
void Gui_Label_Centered(int x, int y, int z, float size, bool shadow, int16_t color, const char* text, ...);
#define BUTTON_HEIGHT 20
#define BUTTON_TEXT_PADDING ((BUTTON_HEIGHT - CHAR_HEIGHT) / 2)
bool Gui_Button(bool enabled,int x, int y, int width, int z, const char* label);
bool Gui_IconButton(int x, int y, int width, int height, const char* label);
bool Gui_IsCursorInside(int x, int y, int w, int h);
bool Gui_WasCursorInside(int x, int y, int w, int h);
void Gui_GetCursorMovement(int* x, int* y);
bool Gui_EnteredCursorInside(int x, int y, int w, int h);

void Gui_DrawOutline(int x, int y, int width, int height, uint32_t color);
void Gui_DrawTint(int x, int y, int width, int height, uint32_t color);
// No, this is not 3D, Z is referring to the Z-Index
// then name it depth
// no fuc off xD
void Gui_DrawBackground(int background, int x, int y, int z);
