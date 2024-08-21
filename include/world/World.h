#pragma once

#include "world/WorkQueue.h"
#include "world/chunk/Chunk.h"

#include "util/math/NumberUtils.h"
#include "util/math/Xorshift.h"
#include "world/storage/WorldInfo.h"
#include <vec/vec.h>

#define CHUNKCACHE_SIZE (9)

#define UNDEADCHUNKS_COUNT (2 * CHUNKCACHE_SIZE + CHUNKCACHE_SIZE * CHUNKCACHE_SIZE)

#define CHUNKPOOL_SIZE (CHUNKCACHE_SIZE * CHUNKCACHE_SIZE + UNDEADCHUNKS_COUNT)

typedef struct Block Block;
typedef struct BiomeGen BiomeGen;

typedef u8 WorldGenType;
enum {
	WorldGen_Default,
	WorldGen_SuperFlat,
	WorldGenTypes_Count
};
typedef u8 Gamemode;
enum {
	Gamemode_Survival,
	Gamemode_Hardcore,
	Gamemode_Creative,
	Gamemode_Adventure,
	Gamemode_Spectator,
	Gamemode_Count
};
typedef u8 Difficulty;
enum {
	Difficulty_Normal,
	Difficulty_Peaceful,
	Difficulty_Easy,
	Difficulty_Hard,
	Difficulty_Count
};
typedef struct {
	WorldGenType type;
	// gamemode type;
	union {
		struct {
			// Keine Einstellungen...
		} superflat;
	} settings;
} GeneratorSettings;

typedef struct {
	bool active;

	WorldInfo worldInfo;

	int HighestBlock;

	GeneratorSettings genSettings;

	int cacheTranslationX, cacheTranslationZ;

	Chunk chunkPool[CHUNKPOOL_SIZE];
	Chunk* chunkCache[CHUNKCACHE_SIZE][CHUNKCACHE_SIZE];
	vec_t(Chunk*) freeChunks;

	Xorshift32 randomTickGen;

	int weather;
} World;

extern World* gWorld;

static inline int WorldToChunkCoord(int x) {
	return (x + (int)(x < 0)) / CHUNK_SIZE - (int)(x < 0);
}
static inline int ChunkToWorldCoord(int x) {
	return (x << 4);
}
static inline int WorldToLocalCoord(int x) {
	return x - (WorldToChunkCoord(x) << 4);
}
static inline int WorldHeightToCluster(int y) {
	return (y >> 4);
}
static inline int ClusterToWorldHeight(int y) {
	return (y << 4);
}
void World_Init();

void World_Deinit();

void World_Tick();

Chunk* World_LoadChunk(int x, int z);
void World_UnloadChunk(Chunk* chunk);

Chunk* World_GetChunk(int x, int z);

BlockId World_GetBlock(int x, int y, int z);
void World_SetBlock(int x, int y, int z, BlockId block);
u8 World_GetBlockMetadata(int x, int y, int z);
void World_SetBlockMetadata(int x, int y, int z, u8 metadata);

void World_SetBlockAndMeta(int x, int y, int z, BlockId block, u8 metadata);

void World_UpdateChunkCache(int orginX, int orginZ);

void World_UpdateChunkGen();

int World_GetChunkHeight(int x, int z);

void World_MarkBlocksForUpdate(int x, int y, int z);

const Material* World_GetBlockMaterial(int x, int y, int z);

BiomeGen* World_GetBiomeGenAt(int x, int y, int z);