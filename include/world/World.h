#pragma once

#include "world/WorkQueue.h"
#include "world/chunk/Chunk.h"

#include "util/math/NumberUtils.h"
#include "util/math/Xorshift.h"
#include <vec/vec.h>

#define CHUNKCACHE_SIZE (9)

#define UNDEADCHUNKS_COUNT (2 * CHUNKCACHE_SIZE + CHUNKCACHE_SIZE * CHUNKCACHE_SIZE)

#define CHUNKPOOL_SIZE (CHUNKCACHE_SIZE * CHUNKCACHE_SIZE + UNDEADCHUNKS_COUNT)

typedef struct Block Block;
typedef struct BiomeGen BiomeGen;

typedef u8 WorldGenType;
enum
{
	WorldGen_Default,
	WorldGen_SuperFlat,
	WorldGenTypes_Count
};
typedef u8 Gamemode;
enum
{
	Gamemode_Survival,
	Gamemode_Hardcore,
	Gamemode_Creative,
	Gamemode_Adventure,
	Gamemode_Spectator,
	Gamemode_Count
};
typedef u8 Difficulty;
enum
{
	Difficulty_Normal,
	Difficulty_Peaceful,
	Difficulty_Easy,
	Difficulty_Hard,
	Difficulty_Count
};
typedef struct {
	u64 seed;
	WorldGenType type;
	// gamemode type;
	union {
		struct {
			// Keine Einstellungen...
		} superflat;
	} settings;
} GeneratorSettings;

#define WORLD_NAME_SIZE 128
typedef struct {
	bool active;

	int HighestBlock;

	char name[WORLD_NAME_SIZE];

	char path[WORLD_NAME_SIZE];

	GeneratorSettings genSettings;

	int cacheTranslationX, cacheTranslationZ;

	Chunk chunkPool[CHUNKPOOL_SIZE];
	Chunk* chunkCache[CHUNKCACHE_SIZE][CHUNKCACHE_SIZE];
	vec_t(Chunk*) freeChunks;

	WorkQueue* workqueue;

	Xorshift32 randomTickGen;

	int weather;
} World;

extern World gWorld;

static inline int WorldToChunkCoord(int x) {
	return (x + (int)(x < 0)) / CHUNK_SIZE - (int)(x < 0);
}
static inline int WorldToLocalCoord(int x) {
	return x - WorldToChunkCoord(x) * CHUNK_SIZE;
}

void World_Init(WorkQueue* workqueue);

void World_Reset();

void World_Tick();

Chunk* World_LoadChunk(int x, int z);
void World_UnloadChunk(Chunk* chunk);

Chunk* World_GetChunk(int x, int z);

BlockId World_GetBlock(int x, int y, int z);
void World_SetBlock(int x, int y, int z, BlockId block);
u8 World_GetMetadata(int x, int y, int z);
void World_SetMetadata(int x, int y, int z, u8 metadata);

void World_SetBlockAndMeta(int x, int y, int z, BlockId block, u8 metadata);

void World_UpdateChunkCache(int orginX, int orginZ);

void World_UpdateChunkGen();

int World_GetHeight(int x, int z);

const Material* World_GetMaterial(int x, int y, int z);

BiomeGen* World_GetBiomeGenAt(int x, int y, int z);