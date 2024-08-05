#pragma once

#include "core/Direction.h"
#include "world/level/biome/BiomeGenType.h"
#include "world/level/block/Blocks.h"
#include "world/level/block/material/Material.h"

#include "client/renderer/VBOCache.h"
#include "util/math/Xorshift.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define CHUNK_SIZE (16)
#define CHUNK_HEIGHT (128)
#define CLUSTER_PER_CHUNK (CHUNK_HEIGHT / CHUNK_SIZE)

typedef struct {
	int y;
	BlockId blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
	u8 metadataLight[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];  // first half metadata, second half light

	u32 revision;

	u16 seeThrough;

	bool empty;
	u32 emptyRevision;

	VBO_Block vbo, transparentVBO;
	size_t vertices, transparentVertices;
	u32 vboRevision;
	bool forceVBOUpdate;
} Cluster;

typedef enum
{
	ChunkGen_Empty,	 //
	ChunkGen_Terrain,
	ChunkGen_Finished  // Terrain | Decoration
} ChunkGenProgress;

typedef struct {
	// Die Gesamtanzahl! >= graphicalTasksRunning
	u32 tasksRunning;
	u32 graphicalTasksRunning;

	u32 uuid;

	ChunkGenProgress genProgress;

	int x, z;
	Cluster clusters[CLUSTER_PER_CHUNK];

	u8 heightmap[CHUNK_SIZE][CHUNK_SIZE];
	u32 heightmapRevision;

	size_t revision;

	u32 displayRevision;
	bool forceVBOUpdate;

	BiomeGenType biome;

	int references;
} Chunk;

extern Xorshift32 uuidGenerator;
extern const u8 _seethroughTable[6][6];
static inline u16 ChunkSeeThrough(Direction in, Direction out) {
	return 1 << (u16)(_seethroughTable[in][out]);
}
static inline bool ChunkCanBeSeenThrough(u16 visiblity, Direction in, Direction out) {
	return visiblity & (1 << (u16)(_seethroughTable[in][out]));
}

static inline void Chunk_Init(Chunk* chunk, int x, int z) {
	memset(chunk, 0, sizeof(Chunk));

	chunk->x = x;
	chunk->z = z;
	for (int i = 0; i < CLUSTER_PER_CHUNK; i++) {
		chunk->clusters[i].y		  = i;
		chunk->clusters[i].seeThrough = UINT16_MAX;
		chunk->clusters[i].empty	  = true;
	}
	chunk->uuid = Xorshift32_Next(&uuidGenerator);

	chunk->biome = BIOME_PLAINS;
}

static inline void Chunk_RequestGraphicsUpdate(Chunk* chunk, int cluster) {
	chunk->clusters[cluster].forceVBOUpdate = true;
	chunk->forceVBOUpdate					= true;
}

void Chunk_GenerateHeightmap(Chunk* chunk);
static inline u8 Chunk_GetHeightMap(Chunk* chunk, int x, int z) {
	Chunk_GenerateHeightmap(chunk);
	return chunk->heightmap[x][z];
}

static inline u8 Chunk_GetMetadata(Chunk* chunk, int x, int y, int z) {
	return chunk->clusters[y / CHUNK_SIZE].metadataLight[x][y - (y / CHUNK_SIZE * CHUNK_SIZE)][z] & 0xf;
}
static inline void Chunk_SetMetadata(Chunk* chunk, int x, int y, int z, u8 metadata) {
	metadata &= 0xf;
	Cluster* cluster = &chunk->clusters[y / CHUNK_SIZE];
	u8* addr		 = &cluster->metadataLight[x][y - (y / CHUNK_SIZE * CHUNK_SIZE)][z];
	*addr			 = (*addr & 0xf0) | metadata;
	++cluster->revision;
	++chunk->revision;
}

static inline BlockId Chunk_GetBlock(Chunk* chunk, int x, int y, int z) {
	return chunk->clusters[y / CHUNK_SIZE].blocks[x][y - (y / CHUNK_SIZE * CHUNK_SIZE)][z];
}
// resets the meta data
static inline void Chunk_SetBlock(Chunk* chunk, int x, int y, int z, BlockId block) {
	Cluster* cluster										 = &chunk->clusters[y / CHUNK_SIZE];
	cluster->blocks[x][y - (y / CHUNK_SIZE * CHUNK_SIZE)][z] = block;
	Chunk_SetMetadata(chunk, x, y, z, 0);
	/*++cluster->revision;
	++chunk->revision;*/  // durch das Setzen der Metadaten wird das sowieso erhöht
}
static inline void Chunk_SetBlockAndMeta(Chunk* chunk, int x, int y, int z, BlockId block, u8 metadata) {
	Cluster* cluster										 = &chunk->clusters[y / CHUNK_SIZE];
	cluster->blocks[x][y - (y / CHUNK_SIZE * CHUNK_SIZE)][z] = block;
	metadata &= 0xf;
	u8* addr = &cluster->metadataLight[x][y - (y / CHUNK_SIZE * CHUNK_SIZE)][z];
	*addr	 = (*addr & 0xf0) | metadata;

	++cluster->revision;
	++chunk->revision;
}
static inline BiomeGenType Chunk_GetBiome(Chunk* chunk) {
	return chunk->biome;
}

bool Cluster_IsEmpty(Cluster* cluster);
