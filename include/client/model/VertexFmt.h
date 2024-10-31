#pragma once

#include <3ds/types.h>
#include <stdint.h>

#include "util/math/NumberUtils.h"
#include "util/math/VecMath.h"

typedef struct {
	float pos[3];
	s16 uv[2];
	u8 rgb[3];
	u8 fxyz[3];
} WorldVertex;

typedef struct {
	s16 pos[3];
	s16 uvc[3];
} GuiVertex;

/*typedef struct {
		s16 xyz[3];
		s16 uv[2];
} CubeMapVertex;*/

#define SHADER_RGB(r, g, b) (((b & 0x1f)) | (((g)&0x1f) << 5) | (((r)&0x1f) << 10))

#define SHADER_R(r) (((r) >> 10) & 0x1f)
#define SHADER_G(g) (((g) >> 5) & 0x1f)
#define SHADER_B(b) ((b)&0x1f)
#define SHADER_RGB_MIX(a, b) SHADER_RGB((SHADER_R(a) + SHADER_R(b)) / 2, (SHADER_G(a) + SHADER_G(b)) / 2, (SHADER_B(a) + SHADER_B(b)) / 2)

#define SHADER_RGB_DARKEN(a, f) SHADER_RGB(((int)SHADER_R(a)) * f / 16, ((int)SHADER_G(a)) * f / 16, ((int)SHADER_B(a)) * f / 16)

s16 HSV16_ToRGB(s16 hsv);
s16 RGB16_ToHSV(u16 rgb);
