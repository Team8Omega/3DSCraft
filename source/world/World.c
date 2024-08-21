#include "world/World.h"

#include "client/Crash.h"
#include "client/gui/DebugUI.h"
#include "world/level/block/BlockEvents.h"

#include "client/player/Player.h"
#include "world/level/block/Block.h"

#include <string.h>

#include <assert.h>
#include <limits.h>
#include <stdint.h>

World* gWorld;

void World_Init() {
	gWorld = (World*)malloc(sizeof(World));

	gWorld->randomTickGen = Xorshift32_New();

	vec_init(&gWorld->freeChunks);

	for (size_t i = 0; i < CHUNKPOOL_SIZE; i++) {
		gWorld->chunkPool[i].x = INT_MAX;
		gWorld->chunkPool[i].z = INT_MAX;
		vec_push(&gWorld->freeChunks, &gWorld->chunkPool[i]);
	}

	Player_Load();
}

void World_Deinit() {
	vec_clear(&gWorld->freeChunks);

	Player_Unload();

	free(gWorld);
	gWorld = NULL;
}

Chunk* World_LoadChunk(int x, int z) {
	for (int i = 0; i < gWorld->freeChunks.length; i++) {
		if (gWorld->freeChunks.data[i]->x == x && gWorld->freeChunks.data[i]->z == z) {
			Chunk* chunk = gWorld->freeChunks.data[i];
			vec_splice(&gWorld->freeChunks, i, 1);

			chunk->references++;
			return chunk;
		}
	}

	for (int i = 0; i < gWorld->freeChunks.length; i++) {
		if (!gWorld->freeChunks.data[i]->tasksRunning) {
			Chunk* chunk = gWorld->freeChunks.data[i];
			vec_splice(&gWorld->freeChunks, i, 1);

			Chunk_Init(chunk, x, z);
			WorkQueue_AddItem((WorkerItem){ WorkerItemType_Load, chunk });
			DebugUI_Text("loading world");
			chunk->references++;
			return chunk;
		}
	}

	return NULL;
}
void World_UnloadChunk(Chunk* chunk) {
	WorkQueue_AddItem((WorkerItem){ WorkerItemType_Save, chunk });
	vec_push(&gWorld->freeChunks, chunk);
	chunk->references--;
}

Chunk* World_GetChunk(int x, int z) {
	int halfS = CHUNKCACHE_SIZE / 2;
	int lowX  = gWorld->cacheTranslationX - halfS;
	int lowZ  = gWorld->cacheTranslationZ - halfS;
	int highX = gWorld->cacheTranslationX + halfS;
	int highZ = gWorld->cacheTranslationZ + halfS;
	if (x >= lowX && z >= lowZ && x <= highX && z <= highZ)
		return gWorld->chunkCache[x - lowX][z - lowZ];
	return NULL;
}

BlockId World_GetBlock(int x, int y, int z) {
	if (y < 0 || y >= CHUNK_HEIGHT)
		return BLOCK_AIR;
	Chunk* chunk = World_GetChunk(WorldToChunkCoord(x), WorldToChunkCoord(z));
	if (chunk)
		return Chunk_GetBlock(chunk, WorldToLocalCoord(x), y, WorldToLocalCoord(z));
	return BLOCK_AIR;
}

#define NOTIFY_NEIGHBOR(axis, comp, xDiff, zDiff)                                                                                          \
	if (axis == comp) {                                                                                                                    \
		Chunk* neighborChunk = World_GetChunk(cX + xDiff, cZ + zDiff);                                                                     \
		if (neighborChunk)                                                                                                                 \
			Chunk_RequestGraphicsUpdate(neighborChunk, cluster);                                                                           \
	}

#define NOTIFY_ALL_NEIGHBORS                                                                                                               \
	u8 cluster = WorldHeightToCluster(y);                                                                                                  \
	NOTIFY_NEIGHBOR(lX, 0, -1, 0);                                                                                                         \
	NOTIFY_NEIGHBOR(lX, 15, 1, 0);                                                                                                         \
	NOTIFY_NEIGHBOR(lZ, 0, 0, -1);                                                                                                         \
	NOTIFY_NEIGHBOR(lZ, 15, 0, 1);                                                                                                         \
	if (WorldToLocalCoord(y) == 0 && cluster != 0)                                                                                         \
		Chunk_RequestGraphicsUpdate(chunk, cluster - 1);                                                                                   \
	if (WorldToLocalCoord(y) == 15 && cluster != CLUSTER_PER_CHUNK)                                                                        \
		Chunk_RequestGraphicsUpdate(chunk, cluster + 1);

void World_SetBlock(int x, int y, int z, BlockId block) {
	if (y < 0 || y >= CHUNK_HEIGHT)
		return;
	int cX		 = WorldToChunkCoord(x);
	int cZ		 = WorldToChunkCoord(z);
	Chunk* chunk = World_GetChunk(cX, cZ);
	if (chunk) {
		int lX = WorldToLocalCoord(x);
		int lZ = WorldToLocalCoord(z);
		Chunk_SetBlock(chunk, lX, y, lZ, block);

		NOTIFY_ALL_NEIGHBORS
	}
}

void World_SetBlockAndMeta(int x, int y, int z, BlockId block, u8 metadata) {
	if (y < 0 || y >= CHUNK_HEIGHT)
		return;
	int cX		 = WorldToChunkCoord(x);
	int cZ		 = WorldToChunkCoord(z);
	Chunk* chunk = World_GetChunk(cX, cZ);
	if (chunk) {
		int lX = WorldToLocalCoord(x);
		int lZ = WorldToLocalCoord(z);
		Chunk_SetBlockAndMeta(chunk, lX, y, lZ, block, metadata);

		NOTIFY_ALL_NEIGHBORS
	}
}

u8 World_GetBlockMetadata(int x, int y, int z) {
	if (y < 0 || y >= CHUNK_HEIGHT)
		return 0;
	Chunk* chunk = World_GetChunk(WorldToChunkCoord(x), WorldToChunkCoord(z));
	if (chunk)
		return Chunk_GetMetadata(chunk, WorldToLocalCoord(x), y, WorldToLocalCoord(z));
	return 0;
}

void World_SetBlockMetadata(int x, int y, int z, u8 metadata) {
	if (y < 0 || y >= CHUNK_HEIGHT)
		return;
	int cX		 = WorldToChunkCoord(x);
	int cZ		 = WorldToChunkCoord(z);
	Chunk* chunk = World_GetChunk(cX, cZ);
	if (chunk) {
		int lX = WorldToLocalCoord(x);
		int lZ = WorldToLocalCoord(z);
		Chunk_SetMetadata(chunk, lX, y, lZ, metadata);

		NOTIFY_ALL_NEIGHBORS
	}
}

int World_GetChunkHeight(int x, int z) {
	int cX		 = WorldToChunkCoord(x);
	int cZ		 = WorldToChunkCoord(z);
	Chunk* chunk = World_GetChunk(cX, cZ);
	if (chunk) {
		int lX = WorldToLocalCoord(x);
		int lZ = WorldToLocalCoord(z);

		return Chunk_GetHeightMap(chunk, lX, lZ);
	}
	return 0;
}

void World_UpdateChunkCache(int orginX, int orginZ) {
	if (orginX != gWorld->cacheTranslationX || orginZ != gWorld->cacheTranslationZ) {
		Chunk* tmpBuffer[CHUNKCACHE_SIZE][CHUNKCACHE_SIZE];
		memcpy(tmpBuffer, gWorld->chunkCache, sizeof(tmpBuffer));

		int oldBufferStartX = gWorld->cacheTranslationX - CHUNKCACHE_SIZE / 2;
		int oldBufferStartZ = gWorld->cacheTranslationZ - CHUNKCACHE_SIZE / 2;

		int diffX = orginX - gWorld->cacheTranslationX;
		int diffZ = orginZ - gWorld->cacheTranslationZ;

		for (int i = 0; i < CHUNKCACHE_SIZE; ++i) {
			for (int j = 0; j < CHUNKCACHE_SIZE; ++j) {
				int wx = orginX + (i - CHUNKCACHE_SIZE / 2);
				int wz = orginZ + (j - CHUNKCACHE_SIZE / 2);
				if (wx >= oldBufferStartX && wx < oldBufferStartX + CHUNKCACHE_SIZE && wz >= oldBufferStartZ &&
					wz < oldBufferStartZ + CHUNKCACHE_SIZE) {
					gWorld->chunkCache[i][j]		= tmpBuffer[i + diffX][j + diffZ];
					tmpBuffer[i + diffX][j + diffZ] = NULL;
				} else {
					gWorld->chunkCache[i][j] = World_LoadChunk(wx, wz);
				}
			}
		}

		for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
			for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
				if (tmpBuffer[i][j] != NULL) {
					World_UnloadChunk(tmpBuffer[i][j]);
				}
			}
		}

		gWorld->cacheTranslationX = orginX;
		gWorld->cacheTranslationZ = orginZ;
	}
}

void World_Tick() {
	World_UpdateChunkGen();
}

void World_UpdateChunkGen() {
	for (int x = 0; x < CHUNKCACHE_SIZE; x++)
		for (int z = 0; z < CHUNKCACHE_SIZE; z++) {
			Chunk* chunk = gWorld->chunkCache[x][z];

			if (chunk->genProgress == ChunkGen_Empty && !chunk->tasksRunning)
				WorkQueue_AddItem((WorkerItem){ WorkerItemType_BaseGen, chunk });

			if (x > 0 && z > 0 && x < CHUNKCACHE_SIZE - 1 && z < CHUNKCACHE_SIZE - 1 && chunk->genProgress == ChunkGen_Terrain &&
				!chunk->tasksRunning) {
				bool clear = true;
				for (int xOff = -1; xOff < 2 && clear; xOff++)
					for (int zOff = -1; zOff < 2 && clear; zOff++) {
						Chunk* borderChunk = gWorld->chunkCache[x + xOff][z + zOff];
						if (borderChunk->genProgress == ChunkGen_Empty || !borderChunk->tasksRunning)
							clear = false;
					}
				if (clear)
					WorkQueue_AddItem((WorkerItem){ WorkerItemType_Decorate, chunk });

				/*int xVals[RANDOMTICKS_PER_CHUNK];
				int yVals[RANDOMTICKS_PER_CHUNK];
				int zVals[RANDOMTICKS_PER_CHUNK];
				for (int i = 0; i < RANDOMTICKS_PER_CHUNK; i++) {
					xVals[i] = WorldToLocalCoord(Xorshift32_Next(&gWorld->randomTickGen));
					yVals[i] = WorldToLocalCoord(Xorshift32_Next(&gWorld->randomTickGen));
					zVals[i] = WorldToLocalCoord(Xorshift32_Next(&gWorld->randomTickGen));
				}*/
			}
		}
}
const Material* World_GetBlockMaterial(int x, int y, int z) {
	return &MATERIALS[BLOCKS[World_GetBlock(x, y, z)]->material];
}
BiomeGen* World_GetBiomeGenAt(int x, int y, int z) {
	Chunk* chunk;
	if (!(chunk = World_GetChunk(WorldToChunkCoord(x), WorldToChunkCoord(z))))
		return NULL;

	return BIOMEGENTYPES[Chunk_GetBiome(chunk)];
}