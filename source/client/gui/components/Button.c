#include "client/gui/components/Button.h"

#include "client/gui/Gui.h"
#include "client/renderer/SpriteBatch.h"

#define SLICE_SIZE 8
#define TEXTURE_Y_OFFSET 66
#define BUTTON_HEIGHT 20

static bool pressed;

Button* Button_Init(u16 x, u16 y, u8 z, u16 width, char* label) {
	Button* b = malloc(sizeof(Button));

	char str[256];
	strcpy(str, label);
	b->label			 = str;
	b->x				 = x;
	b->y				 = y;
	b->buttonFullWidth	 = width;
	b->buttonMiddleWidth = width - (SLICE_SIZE * 2);
	b->textWidth		 = SpriteBatch_CalcTextWidthSingle(label);
	b->visibility		 = Button_VisActive;
	b->pressed			 = false;

	return b;
}

void Button_SetVis(Button* b, ButtonVis vis) {
	b->visibility = vis;
}

void Button_Render(Button* b) {
	if (b->visibility == Button_VisInvisible)
		return;

	SpriteBatch_BindGuiTexture(GuiTexture_Widgets);

	pressed = false;
}