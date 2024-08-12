#pragma once

enum {
	Button_VisActive,
	Button_VisDisabled,
	Button_VisInvisible,
	Button_VisCount
};
typedef u8 ButtonVis;

typedef struct {
	u16 x, y;
	u8 z;
	u16 buttonFullWidth;
	u16 buttonMiddleWidth;
	u16 textWidth;
	ButtonVis visibility;

	const char* label;
	bool pressed;
} Button;

Button* Button_Init(u16 x, u16 y, u8 z, u16 width, char* label);
void Button_SetVis(Button* b, ButtonVis vis);
void Button_Render(Button* b);