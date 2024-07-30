#pragma once

#include <stdint.h>

#include <citro3d.h>

typedef struct {
	u8 fontWidth[1 << 8];
	C3D_Tex texture;
} Font;

void FontLoader_Init(Font* font, const char* filename);
