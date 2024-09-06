#include "client/renderer/WorldRenderer.h"

#include <citro3d.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "world/World.h"
#include "world/level/block/Block.h"

#include "core/VertexFmt.h"

#include "client/player/Player.h"
#include "client/renderer/Clouds.h"
#include "client/renderer/Cursor.h"
#include "client/renderer/Hand.h"

#include "client/Crash.h"
#include "client/gui/DebugUI.h"

int sky_time = 0;

static int projectionUniform;

typedef struct {
	Cluster* cluster;
	Chunk* chunk;
	Direction enteredFrom;
} RenderStep;

typedef struct {
	Cluster* cluster;
	Chunk* chunk;
} TransparentRender;

static vec_t(RenderStep) renderingQueue;
static u8 chunkRendered[CHUNKCACHE_SIZE][CLUSTER_PER_CHUNK][CHUNKCACHE_SIZE];
static vec_t(TransparentRender) transparentClusters;

static C3D_FogLut fogLut;

#define clusterWasRendered(x, y, z)                                                                                                        \
	chunkRendered[x - (gWorld->cacheTranslationX - (CHUNKCACHE_SIZE >> 1))][y][z - (gWorld->cacheTranslationZ - (CHUNKCACHE_SIZE >> 1))]

void WorldRenderer_Init(int projectionUniform_) {
	projectionUniform = projectionUniform_;

	vec_init(&renderingQueue);
	vec_init(&transparentClusters);

	Camera_Init();

	Player_Init();

	Cursor_Init();
	Hand_Init();

	float data[256];
	for (int i = 0; i <= 128; i++) {
		float val = 1.f;
		if (i == 0)
			val = 0.0f;
		else if (i == 1)
			val = 0.9f;
		if (i < 128)
			data[i] = val;
		if (i > 0)
			data[i + 127] = val - data[i - 1];
	}
	FogLut_FromArray(&fogLut, data);

	Clouds_Init();
}
void WorldRenderer_Deinit() {
	vec_deinit(&renderingQueue);
	vec_deinit(&transparentClusters);
	Cursor_Deinit();

	Player_Deinit();

	Hand_Deinit();

	Clouds_Deinit();
}

void WorldRenderer_EnableFog() {
	C3D_FogGasMode(GPU_FOG, GPU_PLAIN_DENSITY, false);
	C3D_FogColor(0xffd990);
	C3D_FogLutBind(&fogLut);
}

static void renderWorld() {
	World_UpdateChunkCache(WorldToChunkCoord(FastFloor(gPlayer->position.x)), WorldToChunkCoord(FastFloor(gPlayer->position.z)));

	memset(chunkRendered, 0, sizeof(chunkRendered));

	u64 polysTotal	  = 0;
	u32 clustersDrawn = 0;

	vec_clear(&renderingQueue);
	vec_clear(&transparentClusters);

	int playerY = CLAMP(WorldHeightToCluster(gPlayer->positionBlock.y), 0, CLUSTER_PER_CHUNK - 1);
	int playerX = WorldToChunkCoord(gPlayer->positionBlock.x);
	int playerZ = WorldToChunkCoord(gPlayer->positionBlock.z);

	Chunk* pChunk = World_GetChunk(playerX, playerZ);

	vec_push(&renderingQueue, ((RenderStep){ &pChunk->clusters[playerY], pChunk, Direction_None }));

	chunkRendered[CHUNKCACHE_SIZE / 2][playerY][CHUNKCACHE_SIZE / 2] = 1;

	float3 playerPos = gPlayer->position;

	C3D_TexBind(0, &gTexMapBlock.texture);

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projectionUniform, &gCamera.vp);
	C3D_FogColor(0xffd990);

	float renderDistance = 3.f;

	while (renderingQueue.length > 0) {
		RenderStep step	 = vec_pop(&renderingQueue);
		Chunk* chunk	 = step.chunk;
		Cluster* cluster = step.cluster;

		if (cluster->vertices > 0 && cluster->vbo.size) {
			clusterWasRendered(chunk->x, cluster->y, chunk->z) |= 2;

			C3D_BufInfo bufInfo;
			BufInfo_Init(&bufInfo);
			BufInfo_Add(&bufInfo, cluster->vbo.memory, sizeof(WorldVertex), 3, 0x3210);
			C3D_SetBufInfo(&bufInfo);
			C3D_DrawArrays(GPU_TRIANGLES, 0, cluster->vertices);

			polysTotal += cluster->vertices;

			clustersDrawn++;
		}
		if (cluster->transparentVertices > 0 && cluster->transparentVBO.size) {
			vec_push(&transparentClusters, ((TransparentRender){ cluster, chunk }));
		}
		// if (polysTotal >= 150000) break;

		for (u8 i = 0; i < 6; ++i) {
			Direction dir	 = i;
			const s8* offset = DirectionToOffset[dir];

			int newX = chunk->x + offset[0], newY = cluster->y + offset[1], newZ = chunk->z + offset[2];
			if (newX < gWorld->cacheTranslationX - (CHUNKCACHE_SIZE >> 1) + 1 ||
				newX > gWorld->cacheTranslationX + (CHUNKCACHE_SIZE >> 1) - 1 ||
				newZ < gWorld->cacheTranslationZ - (CHUNKCACHE_SIZE >> 1) + 1 ||
				newZ > gWorld->cacheTranslationZ + (CHUNKCACHE_SIZE >> 1) - 1 || newY < 0 || newY >= CLUSTER_PER_CHUNK)
				continue;
			float3 dist = f3_sub(f3_new(ChunkToWorldCoord(newX) + (CHUNK_SIZE >> 1), ClusterToWorldHeight(newY) + (CHUNK_SIZE >> 1),
										ChunkToWorldCoord(newZ) + (CHUNK_SIZE >> 1)),
								 playerPos);

			if (f3_dot(dist, dist) > ChunkToWorldCoord(renderDistance) * ChunkToWorldCoord(renderDistance)) {
				continue;
			}

			if (clusterWasRendered(newX, newY, newZ) & 1)
				continue;

			if (!ChunkCanBeSeenThrough(cluster->seeThrough, step.enteredFrom, i) && step.enteredFrom != Direction_None)
				continue;

			C3D_FVec chunkPosition = FVec3_New(ChunkToWorldCoord(newX), ClusterToWorldHeight(newY), ChunkToWorldCoord(newZ));
			if (!Camera_IsAABBVisible(chunkPosition, FVec3_New(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE)))
				continue;

			clusterWasRendered(newX, newY, newZ) |= 1;

			Chunk* newChunk		= World_GetChunk(newX, newZ);
			RenderStep nextStep = (RenderStep){ &newChunk->clusters[newY], newChunk, DirectionOpposite[dir] };
			if (newChunk)
				vec_push(&renderingQueue, nextStep);
		}
	}

	for (int x = 1; x < CHUNKCACHE_SIZE - 1; ++x) {
		for (int z = 1; z < CHUNKCACHE_SIZE - 1; ++z) {
			Chunk* chunk = gWorld->chunkCache[x][z];

			if ((chunk->revision != chunk->displayRevision || chunk->forceVBOUpdate) && !chunk->tasksRunning) {
				bool clear = true;
				for (int xOff = -1; xOff < 2 && clear; ++xOff)
					for (int zOff = -1; zOff < 2 && clear; ++zOff)
						if (gWorld->chunkCache[x + xOff][z + zOff]->genProgress == ChunkGen_Empty)
							clear = false;

				if (clear)
					WorkQueue_AddItem((WorkerItem){ WorkerItemType_PolyGen, chunk });
			}
		}
	}

	C3D_AlphaTest(true, GPU_GEQUAL, 255);

	int i;
	TransparentRender* render;
	vec_foreach_ptr_rev(&transparentClusters, render, i) {
		C3D_BufInfo bufInfo;
		BufInfo_Init(&bufInfo);
		BufInfo_Add(&bufInfo, render->cluster->transparentVBO.memory, sizeof(WorldVertex), 3, 0x3210);
		C3D_SetBufInfo(&bufInfo);
		C3D_DrawArrays(GPU_TRIANGLES, 0, render->cluster->transparentVertices);

		polysTotal += render->cluster->transparentVertices;
	}
	C3D_AlphaTest(false, GPU_GREATER, 0);

	DebugUI_Text("C: %u V: %u D: %u pC: %u, pU: %d", clustersDrawn, polysTotal, (int)renderDistance,
				 gWorld->chunkCache[CHUNKCACHE_SIZE / 2][CHUNKCACHE_SIZE / 2]->tasksRunning, gWorkqueue.queue.length);
}

void WorldRenderer_Tick() {
	World_Tick();
	Clouds_Tick(gPlayer->position.x, gPlayer->position.y, gPlayer->position.z);
}

void WorldRenderer_Render() {
	renderWorld();

	if (gCamera.mode == CameraMode_First) {
		Hand_Draw(projectionUniform, &gCamera.projection, gPlayer->quickSelectBar[gPlayer->quickSelectBarSlot]);
	}
	Player_Draw();

	Clouds_Render(projectionUniform, &gCamera.vp);

	if (gPlayer->blockInActionRange) {
		Cursor_Draw(projectionUniform, &gCamera.vp, gPlayer->viewRayCast.x, gPlayer->viewRayCast.y, gPlayer->viewRayCast.z,
					gPlayer->viewRayCast.direction);
	}
}