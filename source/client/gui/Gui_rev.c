#include "client/gui/Gui_rev.h"

#include <stdarg.h>

#include "client/model/VertexFmt.h"
#include "client/renderer/texture/SpriteBatch.h"
#include "util/math/NumberUtils.h"

static InputData oldInput;
static InputData input;


void Gui_Rev_Init() {
	memset(&input, 0x0, sizeof(InputData));
	memset(&oldInput, 0x0, sizeof(InputData));
}

void Gui_Rev_Deinit() {
}

void Gui_Rev_Label(int x, int y, int z, float size, bool shadow, int16_t color, const char* text, ...) {
	int wrap	  = size <= 0.f ? INFINITY :size;

	va_list vl;
	va_start(vl, text);
    int xTextSize = SpriteBatch_PushTextVargs(x , y, z, color, false, wrap, 0, text, vl);

	va_end(vl);
}

bool Gui_Rev_Button(int x, int y,int width, int height, int z, const char* label) {
	// TODO: Redesign
#define SLICE_SIZE 8
#define textureY 66

    int textWidth = SpriteBatch_CalcTextWidth(label);  // Adjust this call as necessary

    bool pressed = Gui_Rev_IsCursorInside(x, y, width, height);

    int middlePieceSize = width - height * 2;

    SpriteBatch_BindGuiTexture(GuiTexture_Widgets);

    SpriteBatch_PushQuad(x, y, z, SLICE_SIZE, 20, 0, 66 + (pressed * BUTTON_HEIGHT), SLICE_SIZE, 20);
    SpriteBatch_PushQuad(x + SLICE_SIZE, y, z, middlePieceSize, 20, SLICE_SIZE, 66 + (pressed * BUTTON_HEIGHT), middlePieceSize, 20);
    SpriteBatch_PushQuad(x + SLICE_SIZE + middlePieceSize, y, z, SLICE_SIZE, 20, 192, 66 + (pressed * BUTTON_HEIGHT), SLICE_SIZE, 20);

    SpriteBatch_PushText( (x+width/2) - (textWidth / 2),
                          pressed ? (y + (BUTTON_HEIGHT - CHAR_HEIGHT) / 2) + 1 : y + (BUTTON_HEIGHT - CHAR_HEIGHT) / 2, z + 1,
                          pressed ? SHADER_RGB(31, 31, 31) : SHADER_RGB(4, 4, 4), false, INT_MAX, NULL, label);



	if (input.keysup & KEY_TOUCH && Gui_Rev_WasCursorInside(x, y, width, BUTTON_HEIGHT))
		return true;

	return false;
}

bool Gui_Rev_IconButton(int x, int y,int width, int height, const char* label) {
	// TODO: Redesign

	bool pressed = Gui_Rev_IsCursorInside(x, y, width, height);

	SpriteBatch_BindGuiTexture(GuiTexture_Widgets);

	if (pressed) {
		Gui_Rev_DrawTint(x, y, width, height, SHADER_RGB(20, 20, 20));
	}
	SpriteBatch_PushText(x + 24, (y + (height - CHAR_HEIGHT) / 2), -1, SHADER_RGB(31, 31, 31), false, INT_MAX, NULL, label);


	if (input.keysup & KEY_TOUCH && Gui_Rev_WasCursorInside(x, y, width, height))
		return true;

	return false;
}

void Gui_Rev_InputData(InputData data) {
	oldInput = input;
	input	 = data;
}

bool Gui_Rev_IsCursorInside(int x, int y, int w, int h) {
	int sclInputX = input.touchX / SpriteBatch_GetScale();
	int sclInputY = input.touchY / SpriteBatch_GetScale();
	return sclInputX != 0 && sclInputY != 0 && sclInputX >= x && sclInputX < x + w && sclInputY >= y && sclInputY < y + h;
}

bool Gui_Rev_WasCursorInside(int x, int y, int w, int h) {
	int sclOldInputX = oldInput.touchX / SpriteBatch_GetScale();
	int sclOldInputY = oldInput.touchY / SpriteBatch_GetScale();
	return sclOldInputX != 0 && sclOldInputY != 0 && sclOldInputX >= x && sclOldInputX < x + w && sclOldInputY >= y && sclOldInputY < y + h;
}

bool Gui_Rev_EnteredCursorInside(int x, int y, int w, int h) {
	int sclOldInputX = oldInput.touchX / SpriteBatch_GetScale();
	int sclOldInputY = oldInput.touchY / SpriteBatch_GetScale();

	return (sclOldInputX == 0 && sclOldInputY == 0) && Gui_IsCursorInside(x, y, w, h);
}

void Gui_Rev_GetCursorMovement(int* x, int* y) {
	if ((input.touchX == 0 && input.touchY == 0) || (oldInput.touchX == 0 && oldInput.touchY == 0)) {
		*x = 0;
		*y = 0;
		return;
	}
	*x = input.touchX / SpriteBatch_GetScale() - oldInput.touchX / SpriteBatch_GetScale();
	*y = input.touchY / SpriteBatch_GetScale() - oldInput.touchY / SpriteBatch_GetScale();
}

void Gui_Rev_DrawOutline(int x, int y, int width, int height, uint32_t color) {
	// Top border
	SpriteBatch_PushSingleColorQuad(x, y, -3, width, 1, color);
	// Bottom border
	SpriteBatch_PushSingleColorQuad(x, y + height, -3, width + 1, 1, color);
	// Left border
	SpriteBatch_PushSingleColorQuad(x, y, -3, 1, height, color);
	// Right border
	SpriteBatch_PushSingleColorQuad(x + width, y, -3, 1, height, color);
}

void Gui_Rev_DrawTint(int x, int y, int width, int height, uint32_t color) {
	SpriteBatch_PushSingleColorQuad(x, y, -3, width, height, color);
}

void Gui_Rev_DrawBackground(int background, int x, int y, int z) {
	if (background <= 0)
		background = 0;

	if (background == 0) {
		SpriteBatch_BindGuiTexture(GuiTexture_MenuBackground);
		SpriteBatch_PushQuadColor(x * 32, y * 32, z, 32, 32, 0, 0, 16, 16, INT16_MAX);
	}
	if (background == 1) {
		SpriteBatch_BindGuiTexture(GuiTexture_MenuBackground);
		SpriteBatch_PushQuadColor(x * 32, y * 32, z, 32, 32, 16, 0, 16, 16, INT16_MAX);
	}
	if (background == 2) {
		SpriteBatch_BindGuiTexture(GuiTexture_MenuBackground);
		SpriteBatch_PushQuadColor(x * 32, y * 32, z, 32, 32, 0, 16, 16, 16, INT16_MAX);
	}
	if (background == 3) {
		SpriteBatch_BindGuiTexture(GuiTexture_MenuBackground);
		SpriteBatch_PushQuadColor(x * 32, y * 32, z, 32, 32, 16, 16, 16, 16, INT16_MAX);
	}
	if (background >= 3)
		background = 3;
}
