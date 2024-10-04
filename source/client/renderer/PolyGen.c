#include "client/renderer/PolyGen.h"

#include "client/gui/DebugUI.h"
#include "client/renderer/VBOCache.h"
#include "core/Direction.h"
#include "world/level/block/Block.h"
#include "world/level/block/states/BlockStates.h"

#include "client/player/Player.h"

#include <stdbool.h>
#include <vec/vec.h>

#include <3ds.h>

const WorldVertex block_sides_lut[] = {
	// West
	{ { { 0, 0, 0 } }, { 0, 0 }, { 255, 255, 255 } },
	{ { { 0, 0, 16 } }, { 1, 0 }, { 255, 255, 255 } },
	{ { { 0, 16, 16 } }, { 1, 1 }, { 255, 255, 255 } },
	{ { { 0, 16, 16 } }, { 1, 1 }, { 255, 255, 255 } },
	{ { { 0, 16, 0 } }, { 0, 1 }, { 255, 255, 255 } },
	{ { { 0, 0, 0 } }, { 0, 0 }, { 255, 255, 255 } },
	//{ East
	{ { { 16, 0, 0 } }, { 1, 0 }, { 255, 255, 255 } },
	{ { { 16, 16, 0 } }, { 1, 1 }, { 255, 255, 255 } },
	{ { { 16, 16, 16 } }, { 0, 1 }, { 255, 255, 255 } },
	{ { { 16, 16, 16 } }, { 0, 1 }, { 255, 255, 255 } },
	{ { { 16, 0, 16 } }, { 0, 0 }, { 255, 255, 255 } },
	{ { { 16, 0, 0 } }, { 1, 0 }, { 255, 255, 255 } },
	//{ Down
	{ { { 0, 0, 0 } }, { 0, 1 }, { 255, 255, 255 } },
	{ { { 16, 0, 0 } }, { 1, 1 }, { 255, 255, 255 } },
	{ { { 16, 0, 16 } }, { 1, 0 }, { 255, 255, 255 } },
	{ { { 16, 0, 16 } }, { 1, 0 }, { 255, 255, 255 } },
	{ { { 0, 0, 16 } }, { 0, 0 }, { 255, 255, 255 } },
	{ { { 0, 0, 0 } }, { 0, 1 }, { 255, 255, 255 } },
	//{ Up
	{ { { 0, 16, 0 } }, { 0, 1 }, { 255, 255, 255 } },
	{ { { 0, 16, 16 } }, { 0, 0 }, { 255, 255, 255 } },
	{ { { 16, 16, 16 } }, { 1, 0 }, { 255, 255, 255 } },
	{ { { 16, 16, 16 } }, { 1, 0 }, { 255, 255, 255 } },
	{ { { 16, 16, 0 } }, { 1, 1 }, { 255, 255, 255 } },
	{ { { 0, 16, 0 } }, { 0, 1 }, { 255, 255, 255 } },
	//{ North
	{ { { 0, 0, 0 } }, { 1, 0 }, { 255, 255, 255 } },
	{ { { 0, 16, 0 } }, { 1, 1 }, { 255, 255, 255 } },
	{ { { 16, 16, 0 } }, { 0, 1 }, { 255, 255, 255 } },
	{ { { 16, 16, 0 } }, { 0, 1 }, { 255, 255, 255 } },
	{ { { 16, 0, 0 } }, { 0, 0 }, { 255, 255, 255 } },
	{ { { 0, 0, 0 } }, { 1, 0 }, { 255, 255, 255 } },
	//{ South
	{ { { 0, 0, 16 } }, { 0, 0 }, { 255, 255, 255 } },
	{ { { 16, 0, 16 } }, { 1, 0 }, { 255, 255, 255 } },
	{ { { 16, 16, 16 } }, { 1, 1 }, { 255, 255, 255 } },
	{ { { 16, 16, 16 } }, { 1, 1 }, { 255, 255, 255 } },
	{ { { 0, 16, 16 } }, { 0, 1 }, { 255, 255, 255 } },
	{ { { 0, 0, 16 } }, { 0, 0 }, { 255, 255, 255 } },
};

typedef struct {
	VBO_Block vbo, transparentVBO;
	int x, y, z;
	size_t vertices, transparentVertices;
	u8 delay;
	u16 seeThrough;
} VBOUpdate;

static vec_t(VBOUpdate) vboUpdates;

typedef struct {
	s8 x, y, z;
	BlockId block;
	s8 ao;
	u8 metadata;
	bool transparent;
	Direction dir;
} Model;

static inline BlockId fastBlockFetch(Chunk* chunk, Cluster* cluster, int x, int y, int z) {
	return (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE)
			   ? World_GetBlock((chunk->x * CHUNK_SIZE) + x, (cluster->y * CHUNK_SIZE) + y, (chunk->z * CHUNK_SIZE) + z)
			   : cluster->blocks[x][y][z];
}
static inline u8 fastMetadataFetch(Chunk* chunk, Cluster* cluster, int x, int y, int z) {
	return (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE)
			   ? World_GetBlockMetadata((chunk->x * CHUNK_SIZE) + x, (cluster->y * CHUNK_SIZE) + y, (chunk->z * CHUNK_SIZE) + z)
			   : (cluster->metadataLight[x][y][z] & 0xf);
}

typedef struct {
	s8 x, y, z;
} QueueElement;

static vec_t(QueueElement) floodfill_queue;

static LightLock updateLock;

void PolyGen_Init() {
	VBOCache_Init();

	vec_init(&floodfill_queue);

	LightLock_Init(&updateLock);

	vec_init(&vboUpdates);
}
void PolyGen_Deinit() {
	vec_deinit(&vboUpdates);

	VBOCache_Deinit();

	vec_deinit(&floodfill_queue);
}

void PolyGen_Harvest() {
	if (LightLock_TryLock(&updateLock) == 0 && vboUpdates.length > 0 && vboUpdates.data[0].delay++ > 2) {
		DebugUI_Text("VBOUpdates %d", vboUpdates.length);

		while (vboUpdates.length > 0) {
			VBOUpdate update = vec_pop(&vboUpdates);

			Chunk* chunk = World_GetChunk(update.x, update.z);
			if (chunk) {
				if (chunk->clusters[update.y].vertices > 0)
					VBO_Free(chunk->clusters[update.y].vbo);
				if (chunk->clusters[update.y].transparentVertices > 0)
					VBO_Free(chunk->clusters[update.y].transparentVBO);
				chunk->clusters[update.y].vbo				  = update.vbo;
				chunk->clusters[update.y].vertices			  = update.vertices;
				chunk->clusters[update.y].transparentVBO	  = update.transparentVBO;
				chunk->clusters[update.y].transparentVertices = update.transparentVertices;
				chunk->clusters[update.y].seeThrough		  = update.seeThrough;
			}
		}
	}
	LightLock_Unlock(&updateLock);
}

#define MAX_FACES_PER_CLUSTER (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE / 2 * 6)

static Model modelBuffer[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
static Model faceBuffer[MAX_FACES_PER_CLUSTER];
static u32 currentModel;
static u32 currentFace;
static u32 numVertexOpaque;
static u32 numVertexTransparent;
static u8 floodfill_visited[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
static u8 complexMesh_visited[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

static inline void addMesh(bool isSolidBlock, int x, int y, int z, Direction dir, BlockId block, u8 metadata, int ao, bool transparent) {
	if (complexMesh_visited[x][y][z] & 1)
		return;

	if (x >= 0 && y >= 0 && z >= 0 && x < CHUNK_SIZE && y < CHUNK_SIZE && z < CHUNK_SIZE) {
		if (isSolidBlock) {
			faceBuffer[currentFace++] = (Model){ x, y, z, block, ao, metadata, transparent, dir };

			if (!transparent) {
				numVertexOpaque += 6;
			} else {
				numVertexTransparent += 6;
			}

		} else {  // TODO: avoid adding models 6 times per block.
			modelBuffer[currentModel++] = (Model){ x, y, z, block, ao, metadata, transparent, dir };

			if (!transparent) {
				numVertexOpaque += BLOCKSTATES[block].states[metadata].vertexNum;
			} else {
				numVertexTransparent += BLOCKSTATES[block].states[metadata].vertexNum;
			}

			complexMesh_visited[x][y][z] |= 1;
		}
	}
}

static u16 floodFill(Cluster* cluster, int x, int y, int z, Direction entrySide0, Direction entrySide1, Direction entrySide2) {
	if (floodfill_visited[x][y][z] & 1)
		return 0;
	u8 exitPoints[6] = { false };
	if (entrySide0 != Direction_None)
		exitPoints[entrySide0] = true;
	if (entrySide1 != Direction_None)
		exitPoints[entrySide1] = true;
	if (entrySide2 != Direction_None)
		exitPoints[entrySide2] = true;
	vec_clear(&floodfill_queue);
	vec_push(&floodfill_queue, ((QueueElement){ x, y, z }));

	while (floodfill_queue.length > 0) {
		QueueElement item = vec_pop(&floodfill_queue);

		for (int i = 0; i < 6; ++i) {
			const s8* offset = DirectionToOffset[i];
			int x			 = item.x + offset[0];
			int y			 = item.y + offset[1];
			int z			 = item.z + offset[2];
			if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE) {
				exitPoints[i] = true;
			} else {
				if (cluster->blocks[x][y][z] > BLOCK_COUNT - 1)
					World_SetBlock(x, y, z, BLOCK_AIR);

				if ((!(BLOCKS[cluster->blocks[x][y][z]]->opaque) || !BLOCKS[cluster->blocks[x][y][z]]->solidBlock) &&
					!(floodfill_visited[x][y][z] & 1)) {
					floodfill_visited[x][y][z] |= 1;
					vec_push(&floodfill_queue, ((QueueElement){ x, y, z }));
				}

				if ((cluster->blocks[item.x][item.y][item.z] == BLOCK_AIR || BLOCKS[cluster->blocks[x][y][z]]->opaque) &&
					cluster->blocks[x][y][z] != BLOCK_AIR) {
					u8 meta			 = cluster->metadataLight[x][z][z] & 0xf;
					bool transparent = Block_GetMaterial(BLOCKS[cluster->blocks[x][z][z]])->transculent;
					bool solid		 = BLOCKS[cluster->blocks[x][z][z]]->solidBlock;

					addMesh(solid, x, y, z, DirectionOpposite[i], cluster->blocks[x][y][z], meta, 0, transparent);
				}
			}
		}
	}
	u16 seeThrough = 0;
	for (int i = 0; i < 6; ++i)
		if (exitPoints[i])
			for (int j = 0; j < 6; ++j)
				if (i != j && exitPoints[j])
					seeThrough |= ChunkSeeThrough(i, j);
	return seeThrough;
}
static int test;
void PolyGen_GeneratePolygons(WorkerItem item, void* this) {
	test++;
	for (int i = 0; i < CLUSTER_PER_CHUNK; ++i) {
		Cluster* cluster = &item.chunk->clusters[i];

		if (cluster->revision == cluster->vboRevision && !cluster->forceVBOUpdate)
			continue;

		cluster->vboRevision	= cluster->revision;
		cluster->forceVBOUpdate = false;

		currentModel		 = 0;
		currentFace			 = 0;
		numVertexOpaque		 = 0;
		numVertexTransparent = 0;

		u16 seeThrough = 0;

		memset(floodfill_visited, 0, sizeof(floodfill_visited));
		memset(complexMesh_visited, 0, sizeof(complexMesh_visited));
		for (int x = 0; x < CHUNK_SIZE; x += CHUNK_SIZE - 1) {
			Direction xDir = !x ? Direction_West : Direction_East;

			for (int z = 0; z < CHUNK_SIZE; ++z) {
				Direction zDir = z == 0 ? Direction_North : z == CHUNK_SIZE - 1 ? Direction_South : Direction_None;

				for (int y = 0; y < CHUNK_SIZE; ++y) {
					Direction yDir = y == 0 ? Direction_Bottom : y == CHUNK_SIZE - 1 ? Direction_Top : Direction_None;

					Block* block = BLOCKS[cluster->blocks[x][y][z]];
					bool solid	 = block->solidBlock;
					if (!block->opaque || !solid)
						seeThrough |= floodFill(cluster, x, y, z, xDir, yDir, zDir);

					Block* blockNeighbor = BLOCKS[fastBlockFetch(item.chunk, cluster, x + (!x ? -1 : 1), y, z)];

					if (!blockNeighbor->opaque && cluster->blocks[x][y][z] != BLOCK_AIR) {
						addMesh(solid, x, y, z, xDir, cluster->blocks[x][y][z], cluster->metadataLight[x][y][z] & 0xf, 0,
								Block_GetMaterial(block)->transculent);
					}
				}
			}
		}
		for (int y = 0; y < CHUNK_SIZE; y += CHUNK_SIZE - 1) {
			Direction yDir = !y ? Direction_Bottom : Direction_Top;

			for (int x = 0; x < CHUNK_SIZE; ++x) {
				Direction xDir = x == 0 ? Direction_West : x == CHUNK_SIZE - 1 ? Direction_East : Direction_None;

				for (int z = 0; z < CHUNK_SIZE; ++z) {
					Direction zDir = z == 0 ? Direction_South : x == CHUNK_SIZE - 1 ? Direction_North : Direction_None;

					Block* block = BLOCKS[cluster->blocks[x][y][z]];
					bool solid	 = block->solidBlock;
					if (!block->opaque || !solid)
						seeThrough |= floodFill(cluster, x, y, z, xDir, yDir, zDir);

					Block* blockNeighbor = BLOCKS[fastBlockFetch(item.chunk, cluster, x, y + (!y ? -1 : 1), z)];

					if (!blockNeighbor->opaque && cluster->blocks[x][y][z] != BLOCK_AIR) {
						addMesh(solid, x, y, z, yDir, cluster->blocks[x][y][z], cluster->metadataLight[x][y][z] & 0xf, 0,
								Block_GetMaterial(block)->transculent);
					}
				}
			}
		}
		for (int z = 0; z < CHUNK_SIZE; z += CHUNK_SIZE - 1) {
			Direction zDir = !z ? Direction_North : Direction_South;

			for (int x = 0; x < CHUNK_SIZE; ++x) {
				Direction xDir = x == 0 ? Direction_West : x == CHUNK_SIZE - 1 ? Direction_East : Direction_None;

				for (int y = 0; y < CHUNK_SIZE; ++y) {
					Direction yDir = y == 0 ? Direction_Bottom : y == CHUNK_SIZE ? Direction_Top : Direction_None;

					Block* block = BLOCKS[cluster->blocks[x][y][z]];
					bool solid	 = block->solidBlock;
					if (!block->opaque || !solid)
						seeThrough |= floodFill(cluster, x, y, z, xDir, yDir, zDir);

					Block* blockNeighbor = BLOCKS[fastBlockFetch(item.chunk, cluster, x, y, z + (!z ? -1 : 1))];

					if (!blockNeighbor->opaque && cluster->blocks[x][y][z] != BLOCK_AIR) {
						addMesh(solid, x, y, z, zDir, cluster->blocks[x][y][z], cluster->metadataLight[x][y][z] & 0xf, 0,
								Block_GetMaterial(block)->transculent);
					}
				}
			}
		}
		int px = WorldToChunkCoord(FastFloor(gPlayer->position.x)), py = WorldToChunkCoord(FastFloor(gPlayer->position.y)),
			pz = WorldToChunkCoord(FastFloor(gPlayer->position.z));
		if (px == item.chunk->x && pz == item.chunk->z && py == i) {
			floodFill(cluster, px, py, pz, 0, 0, 0);
		}

		VBOUpdate update;

		if (currentModel || currentFace) {
			VBO_Block opaqueMem;
			if (numVertexOpaque > 0) {
				opaqueMem = VBO_Alloc(numVertexOpaque * sizeof(WorldVertex));

				if (!opaqueMem.memory)
					return Crash(0, "Failed Allocating VBO Opaque %d %d", numVertexOpaque, currentFace);
			}

			VBO_Block transparentMem;
			if (numVertexTransparent > 0) {
				transparentMem = VBO_Alloc(numVertexTransparent * sizeof(WorldVertex));

				if (!transparentMem.memory)
					return Crash(0, "Failed Allocating VBO Transparent");
			}

			WorldVertex* opaqueData		 = opaqueMem.memory;
			WorldVertex* transparentData = transparentMem.memory;

			for (int j = 0; j < currentFace; ++j) {
				Model face = faceBuffer[j];

				int offsetX = face.x + item.chunk->x * CHUNK_SIZE;
				int offsetZ = face.z + item.chunk->z * CHUNK_SIZE;
				int offsetY = face.y + i * CHUNK_SIZE;

				WorldVertex* data = face.transparent ? transparentData : opaqueData;

				BlockStateVariant* blockVar;

				if (BLOCKS[face.block]->hasRandomVariants) {
					WeightedRandom* random = BLOCKSTATES[face.block].states[face.metadata].random;
					blockVar			   = &BLOCKSTATES[face.block].states[face.metadata].variants[WeightedRandom_GetRandom(random)];
				} else {
					blockVar = &BLOCKSTATES[face.block].states[face.metadata].variants[0];
				}

				memcpy(data, (blockVar->model->vertex + face.dir * 6), sizeof(WorldVertex) * 6);

				for (int k = 0; k < 6; ++k) {
					const float3* pos = &block_sides_lut[face.dir * 6 + k].pos;

					data[k].pos.x = pos->x + (offsetX << 4);
					data[k].pos.y = pos->y + (offsetY << 4);
					data[k].pos.z = pos->z + (offsetZ << 4);
				}
				if (face.transparent)
					transparentData += 6;
				else
					opaqueData += 6;
			}
			for (int j = 0; j < currentModel; ++j) {
				Crash(0, "You added models? If not, dont set isSolidBlock to false. DEV INFO, REPORT!");
				Model face = modelBuffer[j];

				int offsetX = face.x + item.chunk->x * CHUNK_SIZE;
				int offsetZ = face.z + item.chunk->z * CHUNK_SIZE;
				int offsetY = face.y + i * CHUNK_SIZE;

				WorldVertex* data = face.transparent ? transparentData : opaqueData;

				BlockStateVariant* blockVar;

				if (BLOCKS[face.block]->hasRandomVariants) {
					WeightedRandom* random = BLOCKSTATES[face.block].states[face.metadata].random;
					blockVar			   = &BLOCKSTATES[face.block].states[face.metadata].variants[WeightedRandom_GetRandom(random)];
				} else {
					blockVar = &BLOCKSTATES[face.block].states[face.metadata].variants[0];
				}

				size_t size = blockVar->model->numVertex;

				if (size == 0)
					continue;

				memcpy(data, blockVar->model->vertex, sizeof(WorldVertex) * size);

				for (int k = 0; k < size; ++k) {
					data[k].pos.x += offsetX << 4;
					data[k].pos.y += offsetY << 4;
					data[k].pos.z += offsetZ << 4;
				}
				if (face.transparent)
					transparentData += size;
				else
					opaqueData += size;
			}

			update.vbo			  = opaqueMem;
			update.transparentVBO = transparentMem;
		}

		update.x				   = item.chunk->x;
		update.y				   = i;
		update.z				   = item.chunk->z;
		update.vertices			   = numVertexOpaque;
		update.delay			   = 0;
		update.seeThrough		   = seeThrough;
		update.transparentVertices = numVertexTransparent;

		LightLock_Lock(&updateLock);
		vec_push(&vboUpdates, update);
		LightLock_Unlock(&updateLock);
	}
	item.chunk->displayRevision = item.chunk->revision;
	item.chunk->forceVBOUpdate	= false;
}
