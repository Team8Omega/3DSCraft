#include "world/GrassColors.h"

#include "client/Crash.h"
#include "client/renderer/texture/TextureMap.h"

#define GRASSMAP_SIZE 256
#define TEX_PATH "colormap/grass.png"

C3D_Tex colormap;
static u32 grassBuffer[GRASSMAP_SIZE * GRASSMAP_SIZE];

#define colR(c) ((c >> 16) & 0xff)
#define colG(c) (((c) >> 8) & 0xff)
#define colB(c) ((c) & 0xff)

#include <stdio.h>
#include <string.h>

#ifdef NOPE
void GrassColors_DebugToFile() {
	FILE* file = fopen("sdmc:/3dscraft/debug.txt", "w");
	if (!file) {
		Crash(0, "Failed to open debug file for writing!");
		return;
	}

	for (int y = 0; y < GRASSMAP_SIZE; ++y) {
		for (int x = 0; x < GRASSMAP_SIZE; ++x) {
			u32 color = grassBuffer[y * GRASSMAP_SIZE + x];
			u8 r	  = colR(color);
			u8 g	  = colG(color);
			u8 b	  = colB(color);

			fprintf(file, "Color at (%d, %d): R=%d, G=%d, B=%d, Color ID=%lu\n", x, y, r, g, b, (unsigned long)color);
		}
	}

	fclose(file);
}
#endif

void GrassColors_Init() {
	Texture_Load(&colormap, TEX_PATH);

	u32* data = (u32*)colormap.data;
	for (u32 i = 0; i < GRASSMAP_SIZE * GRASSMAP_SIZE; ++i) {
		u32 pixel = data[i];
		u8 r	  = pixel & 0xFF;
		u8 g	  = (pixel >> 8) & 0xFF;
		u8 b	  = (pixel >> 16) & 0xFF;

		// Check if the color is not pure white (255, 255, 255)
		if (!(r == 255 && g == 255 && b == 255)) {
			r = 255 - r;
			g = 255 - g;
			b = 255 - b;
		}

		grassBuffer[i] = (r) | (g << 8) | (b << 16);
	}
	// GrassColors_DebugToFile();
}

#define colR(c) ((c >> 16) & 0xff)
#define colG(c) (((c) >> 8) & 0xff)
#define colB(c) ((c) & 0xff)

u32 GrassColors_Get(float temp, float rain) {
	rain *= temp;
	int var4 = (int)((1.f - temp) * 255.f);
	int var5 = (int)((1.f - rain) * 255.f);
	return grassBuffer[var5 << 8 | var4];
}
