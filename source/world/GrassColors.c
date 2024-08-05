#include "world/GrassColors.h"

#include "client/Crash.h"
#include "client/renderer/texture/TextureMap.h"

#define GRASSMAP_SIZE 256
#define TEX_PATH "colormap/grass.png"

C3D_Tex colormap;
static u32 grassBuffer[GRASSMAP_SIZE * GRASSMAP_SIZE];

void GrassColors_Init() {
	Texture_Load(&colormap, TEX_PATH);

	u32* data = (u32*)colormap.data;
	for (u32 i = 0; i < GRASSMAP_SIZE * GRASSMAP_SIZE; ++i) {
		u32 pixel = data[i];
		u8 r	  = pixel & 0xFF;
		u8 g	  = (pixel >> 8) & 0xFF;
		u8 b	  = (pixel >> 16) & 0xFF;

		grassBuffer[i] = (r) | (g << 8) | (b << 16);
	}
}
#define colR(c) ((c >> 16) & 0xff)
#define colG(c) (((c) >> 8) & 0xff)
#define colB(c) ((c)&0xff)

u32 GrassColors_Get(float temp, float rain) {
	rain *= temp;
	int var4 = (int)((1.f - temp) * 255.f);
	int var5 = (int)((1.f - rain) * 255.f);
	return grassBuffer[var5 << 8 | var4];
}