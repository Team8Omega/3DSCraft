#pragma once

#include "world/WorkQueue.h"
#include "world/chunk/Chunk.h"

#include "util/math/NumberUtils.h"
#include "util/math/Xorshift.h"
#include <vec/vec.h>

#define CHUNKCACHE_SIZE (9)

#define UNDEADCHUNKS_COUNT (2 * CHUNKCACHE_SIZE + CHUNKCACHE_SIZE * CHUNKCACHE_SIZE)

#define CHUNKPOOL_SIZE (CHUNKCACHE_SIZE * CHUNKCACHE_SIZE + UNDEADCHUNKS_COUNT)

typedef enum {
	WorldGen_Default,
	WorldGen_SuperFlat,
	WorldGenTypes_Count
} WorldGenType;
typedef enum {
	Gamemode_Survival,
	Gamemode_Creative,
	Gamemode_Adventure,
	Gamemode_Spectator,
	Gamemode_Count
} Gamemode;
typedef enum {
    Difficulty_Normal,
    Difficulty_Peaceful,
    Difficulty_Easy,
    Difficulty_Hard,
    Difficulty_Hardcore,
    Difficulty_Count
} Difficulty;
typedef struct {
		uint64_t seed;
		WorldGenType type;
		// gamemode type;
		union {
				struct {
						// Keine Einstellungen...
				} superflat;
		} settings;
} GeneratorSettings;

#define WORLD_NAME_SIZE 256
typedef struct {
		int HighestBlock;

		char name[WORLD_NAME_SIZE];

		GeneratorSettings genSettings;

		int cacheTranslationX, cacheTranslationZ;

		Chunk chunkPool[CHUNKPOOL_SIZE];
		Chunk* chunkCache[CHUNKCACHE_SIZE][CHUNKCACHE_SIZE];
		vec_t(Chunk*) freeChunks;

		WorkQueue* workqueue;

		Xorshift32 randomTickGen;

		int weather;
} World;

static inline int WorldToChunkCoord(int x) {
	return (x + (int)(x < 0)) / CHUNK_SIZE - (int)(x < 0);
}
static inline int WorldToLocalCoord(int x) {
	return x - WorldToChunkCoord(x) * CHUNK_SIZE;
}

void World_Init(World* world, WorkQueue* workqueue);

void World_Reset(World* world);

void World_Tick(World* world);

Chunk* World_LoadChunk(World* world, int x, int z);
void World_UnloadChunk(World* world, Chunk* chunk);

Chunk* World_GetChunk(World* world, int x, int z);

Block World_GetBlock(World* world, int x, int y, int z);
void World_SetBlock(World* world, int x, int y, int z, Block block);
uint8_t World_GetMetadata(World* world, int x, int y, int z);
void World_SetMetadata(World* world, int x, int y, int z, uint8_t metadata);

void World_SetBlockAndMeta(World* world, int x, int y, int z, Block block, uint8_t metadata);

void World_UpdateChunkCache(World* world, int orginX, int orginZ);

int World_GetHeight(World* world, int x, int z);
