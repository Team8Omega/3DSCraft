#pragma once

#include <c3d/texture.h>
#include <stdint.h>

void Texture_Load(C3D_Tex* result, const char* filename);

#define TEXTURE_MAPSIZE 128
#define TEXTURE_TILESIZE 16
#define TEXTURE_MAPTILES (TEXTURE_MAPSIZE / TEXTURE_TILESIZE)

typedef struct {
	s16 u, v;
} Icon;

typedef struct {
	C3D_Tex texture;
	Icon icons[TEXTURE_MAPTILES * TEXTURE_MAPTILES];
} Texture_Map;

u16 Texture_MapAdd(const char* path);
void Texture_MapInit(Texture_Map* map);
// Icon Texture_MapGetIcon(Texture_Map* map, char* filename);
void Texture_MapDeinit(Texture_Map* map);

void Texture_TileImage8(u8* src, u8* dst, int size);

extern Texture_Map gTexMapBlock;