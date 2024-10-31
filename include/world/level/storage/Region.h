#pragma once

#include "world/chunk/Chunk.h"

#include <vec/vec.h>

#define REGION_SIZE 16
#define REGION_BLOCKSIZE (REGION_SIZE * CHUNK_SIZE)

typedef struct {
	u32 position;
	u32 compressedSize;
	u32 actualSize;
	u8 blockSize;
	u32 revision;
} ChunkInfo;

typedef struct {
	int x, z;
	FILE* dataFile;
	ChunkInfo grid[REGION_SIZE][REGION_SIZE];
	vec_t(u8) sectors;
} Region;

static inline int ChunkToRegionCoord(int x) {
	return (x + (int)(x < 0)) / REGION_SIZE - (int)(x < 0);
}
static inline int ChunkToLocalRegionCoord(int x) {
	return x - ChunkToRegionCoord(x) * REGION_SIZE;
}

void Region_InitPools();
void Region_DeinitPools();

void Region_Init(Region* region, int x, int z);
void Region_Deinit(Region* region);
void Region_SaveIndex(Region* region);

void Region_SaveChunk(Region* region, Chunk* chunk);
void Region_LoadChunk(Region* region, Chunk* chunk);
