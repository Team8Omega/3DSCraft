#include "client/renderer/texture/TextureMap.h"

#include <lodepng/lodepng.h>

#include <3ds.h>
#include <citro3d.h>

#include <dirent.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client/Crash.h"
#include "client/gui/DebugUI.h"
#include "util/Paths.h"
#include "util/StringUtils.h"

static C3D_Tex sTexError;

u32 hash(const char* str) {
	unsigned long hash = 5381;
	int c;
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	return hash;
}

void tileImage32(u32* src, u8* dst, int sizex, int sizez);

void Texture_Init() {
	Texture_Load(&sTexError, "romfs:/error.png");
}

void Texture_Load(C3D_Tex* result, const char* filename) {
	const char* filepath = filename;
	if (access(filename, F_OK))
		filepath = String_ParsePackName(PACK_VANILLA, PATH_PACK_TEXTURES, filename);

	u32* image		   = NULL;
	unsigned int width = 255, height = 255;
	u32 error = lodepng_decode32_file((u8**)&image, &width, &height, filepath);
	if (error == 0 && image != NULL) {
		u32* imgInLinRam = (u32*)linearAlloc(width * height * sizeof(u32));

		if (width < 64 || height < 64) {
			for (int j = 0; j < height; j++)
				for (int i = 0; i < width; i++) {
					image[i + j * width] = __builtin_bswap32(image[i + j * width]);
				}
		} else {
			for (int i = 0; i < width * height; i++) {
				/*u32 r = ((image[i] << 0) & 0xff) >> 4;
				u32 g = ((image[i] << 8) & 0xff) >> 4;
				u32 b = ((image[i] << 16) & 0xff) >> 4;
				u32 a = ((image[i] << 24) & 0xff) >> 4;
				imgInLinRam[i] = r | (g << 4) | (b << 8) | (a << 12);*/
				imgInLinRam[i] = __builtin_bswap32(image[i]);
			}
		}

		C3D_TexInitVRAM(result, width, height, GPU_RGBA8);

		if (width < 64 || height < 64)
			tileImage32(image, (u8*)imgInLinRam, width, height);

		GSPGPU_FlushDataCache(imgInLinRam, width * height * sizeof(u32));
		free(image);

		C3D_TexSetFilter(result, GPU_NEAREST, GPU_NEAREST);

		if (width < 64 || height < 64) {
			C3D_SyncTextureCopy(imgInLinRam, 0, result->data, 0, width * height * sizeof(u32), 8);
		} else {
			C3D_SyncDisplayTransfer((u32*)imgInLinRam, GX_BUFFER_DIM(width, height), result->data, GX_BUFFER_DIM(width, height),
									(GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) |
									 GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) |
									 GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO)));
		}

		linearFree(imgInLinRam);
	} else {
		// 83 = alloc
		//
#ifdef DEBUG_LOG
		if (error != 0) {
			char name[512];
			strcpy(name, filename);
			DebugUI_Log("Failed to load texture %s: Code %d", name, error);
		}
#endif

		if (error == 83) {
			Crash(CRASH_ALLOC, "Allocation Error at Texture_Load\nCould not allocate space for '%s'", filename);
		}

		memcpy(result, &sTexError, sizeof(C3D_Tex));
		result->data = linearAlloc(sTexError.size);
		if (!result->data)
			Crash(0, "ahem.");
		memcpy(result->data, sTexError.data, sTexError.size);
	}
}

// Grabbed from Citra Emulator (citra/src/video_core/utils.h)
static inline u32 morton_interleave(u32 x, u32 y) {
	u32 i = (x & 7) | ((y & 7) << 8);  // ---- -210
	i	  = (i ^ (i << 2)) & 0x1313;   // ---2 --10
	i	  = (i ^ (i << 1)) & 0x1515;   // ---2 -1-0
	i	  = (i | (i >> 7)) & 0x3F;
	return i;
}
// Grabbed from Citra Emulator (citra/src/video_core/utils.h)
static inline u32 get_morton_offset(u32 x, u32 y, u32 bytes_per_pixel) {
	u32 i				= morton_interleave(x, y);
	unsigned int offset = (x & ~7) * 8;
	return (i + offset) * bytes_per_pixel;
}
// from sf2d https://github.com/xerpi/sf2dlib/blob/effe77ea81d21c26bad457d4f5ed8bb16ce7b753/libsf2d/source/sf2d_texture.c
void tileImage32(u32* src, u8* dst, int sizex, int sizey) {
	for (int j = 0; j < sizey; j++) {
		for (int i = 0; i < sizex; i++) {
			u32 coarse_y   = j & ~7;
			u32 dst_offset = get_morton_offset(i, j, 4) + coarse_y * sizex * 4;

			u32 v					  = src[i + (sizey - 1 - j) * sizex];
			*(u32*)(dst + dst_offset) = v;
		}
	}
}
void Texture_TileImage8(u8* src, u8* dst, int size) {
	for (int j = 0; j < size; j++) {
		for (int i = 0; i < size; i++) {
			u32 coarse_y   = j & ~7;
			u32 dst_offset = get_morton_offset(i, j, 1) + coarse_y * size;

			u8 v				= src[i + (size - 1 - j) * size];
			*(dst + dst_offset) = v;
		}
	}
}
void downscaleImage(u8* data, int size) {
	int i, j;
	for (j = 0; j < size; j++) {
		for (i = 0; i < size; i++) {
			const u32 offset  = (i + j * size) * 4;
			const u32 offset2 = (i * 2 + j * 2 * size * 2) * 4;
			data[offset + 0]  = (data[offset2 + 0 + 0] + data[offset2 + 4 + 0] + data[offset2 + size * 4 * 2 + 0] +
								 data[offset2 + (size * 2 + 1) * 4 + 0]) /
							   4;
			data[offset + 1] = (data[offset2 + 0 + 1] + data[offset2 + 4 + 1] + data[offset2 + size * 4 * 2 + 1] +
								data[offset2 + (size * 2 + 1) * 4 + 1]) /
							   4;
			data[offset + 2] = (data[offset2 + 0 + 2] + data[offset2 + 4 + 2] + data[offset2 + size * 4 * 2 + 2] +
								data[offset2 + (size * 2 + 1) * 4 + 2]) /
							   4;
			data[offset + 3] = (data[offset2 + 0 + 3] + data[offset2 + 4 + 3] + data[offset2 + size * 4 * 2 + 3] +
								data[offset2 + (size * 2 + 1) * 4 + 3]) /
							   4;
		}
	}
}

static char pathCollect[TEXTURE_TILENUM][512];
static u32 hashCollect[TEXTURE_TILENUM];
static size_t pathNum = 0;

void Texture_MapAddName(const char* path, int out_uv[]) {
	if (pathNum >= TEXTURE_TILENUM)
		Crash(0, "Too many entries for TextureMap\n - you know what this means.");

	// check if already loaded
	u32 hash = String_Hash(path);
	for (size_t i = 0; i < pathNum; ++i) {
		if (hashCollect[i] == hash) {
			out_uv[0] = ((i % TEXTURE_MAPTILES) << 4);
			out_uv[1] = ((i / TEXTURE_MAPTILES) << 4);
			return;
		}
	}

	size_t index = pathNum;

	out_uv[0] = ((index % TEXTURE_MAPTILES) << 4);
	out_uv[1] = ((index / TEXTURE_MAPTILES) << 4);

	pathNum++;
	strcpy(pathCollect[index], path);
	hashCollect[index] = String_Hash(path);
}

void Texture_MapInit(Texture_Map* map) {
	int locX = 0;
	int locY = 0;

	const int mipmapLevels = 2;
	const int maxSize	   = 4 * TEXTURE_MAPSIZE * TEXTURE_MAPSIZE;

	u32* buffer = (u32*)linearAlloc(maxSize);
	for (int i = 0; i < maxSize; i++)
		buffer[i] = 0x000000FF;

	int filei			 = 0;
	const char* filename = pathCollect[0];
	int c				 = 0;
	while (filename != NULL && c < (TEXTURE_TILENUM) && filei < pathNum) {
		if (access(filename, F_OK))
			filename = String_ParsePackName(PACK_VANILLA, PATH_PACK_TEXTURES, String_AddSuffix(filename, ".png"));

		u32* image;
		unsigned int w, h;
		u32 error = lodepng_decode32_file((u8**)&image, &w, &h, filename);
		if (w == TEXTURE_TILESIZE && h == TEXTURE_TILESIZE && image != NULL && error == 0) {
			for (int x = 0; x < TEXTURE_TILESIZE; ++x) {
				for (int y = 0; y < TEXTURE_TILESIZE; ++y) {
					buffer[(locX + x) + ((y + locY) * TEXTURE_MAPSIZE)] =
						__builtin_bswap32(image[x + ((TEXTURE_TILESIZE - y - 1) * TEXTURE_TILESIZE)]);
				}
			}

			Icon* icon = &map->icons[c];
			icon->u	   = locX << 8;
			icon->v	   = locY << 8;

			locX += TEXTURE_TILESIZE;
			if (locX == TEXTURE_MAPSIZE) {
				locY += TEXTURE_TILESIZE;
				locX = 0;
			}
		} else {
#ifdef DEBUG_LOG
			if (error != 0) {
				char name[512];
				strcpy(name, filename);
				DebugUI_Log("Failed to load texture %s: Code %d", name, error);
			}
#endif
			map->icons[c].u = 0;
			map->icons[c].v = 0;
		}
		free(image);
		filename = pathCollect[++filei];
		c++;
	}

	GSPGPU_FlushDataCache(buffer, maxSize);
	if (!C3D_TexInitWithParams(&map->texture, NULL,
							   (C3D_TexInitParams){ TEXTURE_MAPSIZE, TEXTURE_MAPSIZE, mipmapLevels, GPU_RGBA8, GPU_TEX_2D, true }))
		printf("Couldn't alloc texture memory\n");
	C3D_TexSetFilter(&map->texture, GPU_NEAREST, GPU_NEAREST);

	C3D_SyncDisplayTransfer(
		buffer, GX_BUFFER_DIM(TEXTURE_MAPSIZE, TEXTURE_MAPSIZE), map->texture.data, GX_BUFFER_DIM(TEXTURE_MAPSIZE, TEXTURE_MAPSIZE),
		(GX_TRANSFER_FLIP_VERT(1) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |
		 GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO)));

	int size		 = TEXTURE_MAPSIZE / 2;
	ptrdiff_t offset = TEXTURE_MAPSIZE * TEXTURE_MAPSIZE;

	u32* tiledImage = linearAlloc(size * size * 4);

	for (int i = 0; i < mipmapLevels; i++) {
		downscaleImage((u8*)buffer, size);

		tileImage32(buffer, (u8*)tiledImage, size, size);

		GSPGPU_FlushDataCache(tiledImage, size * size * 4);

		GX_RequestDma(tiledImage, ((u32*)map->texture.data) + offset, size * size * 4);
		gspWaitForAnyEvent();

		offset += size * size;
		size /= 2;
	}

	pathNum = 0;

	linearFree(tiledImage);
	linearFree(buffer);
}
void Texture_MapDeinit(Texture_Map* map) {
	C3D_TexDelete(&map->texture);
}

/*
Icon Texture_MapGetIcon(Texture_Map* map, char* filename) {
	u32 h = hash(filename);
	for (size_t i = 0; i < TEXTURE_TILENUM; i++) {
		if (h == map->icons[i].textureHash) {
			return map->icons[i];
		}
	}
	return (Icon){ 0 };
}
*/