#include "client/renderer/PolyGen.h"

#include "client/gui/DebugUI.h"
#include "client/renderer/VBOCache.h"
#include "core/Direction.h"
#include "world/level/block/Block.h"

#include "client/player/Player.h"

#include <stdbool.h>
#include <vec/vec.h>

#include <3ds.h>

const WorldVertex block_sides_lut[] = {
	// Fourth face (MX) - West
	// First triangle
	{ { 0, 0, 0 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { 0, 0, 1 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 0, 1, 1 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	// Second triangle
	{ { 0, 1, 1 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 0, 1, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 0, 0, 0 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	// Third face (PX) - East
	// First triangle
	{ { 1, 0, 0 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 1, 1, 0 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 1, 1, 1 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	// Second triangle
	{ { 1, 1, 1 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 1, 0, 1 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { 1, 0, 0 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	// Sixth face (MY) - Down
	// First triangle
	{ { 0, 0, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 1, 0, 0 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 1, 0, 1 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	// Second triangle
	{ { 1, 0, 1 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 0, 0, 1 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { 0, 0, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	// Fifth face (PY) - Up
	// First triangle
	{ { 0, 1, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 0, 1, 1 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { 1, 1, 1 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	// Second triangle
	{ { 1, 1, 1 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 1, 1, 0 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 0, 1, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	// Second face (MZ) - North
	// First triangle
	{ { 0, 0, 0 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 0, 1, 0 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 1, 1, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	// Second triangle
	{ { 1, 1, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 1, 0, 0 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { 0, 0, 0 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	// First face (PZ) - South
	// First triangle
	{ { 0, 0, 1 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { 1, 0, 1 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 1, 1, 1 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	// Second triangle
	{ { 1, 1, 1 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 0, 1, 1 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 0, 0, 1 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
};

typedef struct {
	VBO_Block vbo, transparentVBO;
	int x, y, z;
	size_t vertices, transparentVertices;
	u8 delay;
	u16 seeThrough;
} VBOUpdate;

static vec_t(VBOUpdate) vboUpdates;

#define MAX_FACES_PER_CLUSTER (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE / 2 * 6)

typedef struct {
	s8 x, y, z;
	Direction direction;
	BlockId block;
	s8 ao;
	u8 metadata;
	bool transparent;
} Face;

static inline BlockId fastBlockFetch(Chunk* chunk, Cluster* cluster, int x, int y, int z) {
	return (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE)
			   ? World_GetBlock((chunk->x * CHUNK_SIZE) + x, (cluster->y * CHUNK_SIZE) + y, (chunk->z * CHUNK_SIZE) + z)
			   : cluster->blocks[x][y][z];
}
static inline u8 fastMetadataFetch(Chunk* chunk, Cluster* cluster, int x, int y, int z) {
	return (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE)
			   ? World_GetMetadata((chunk->x * CHUNK_SIZE) + x, (cluster->y * CHUNK_SIZE) + y, (chunk->z * CHUNK_SIZE) + z)
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

static Face faceBuffer[MAX_FACES_PER_CLUSTER];
static int currentFace;
static int transparentFaces;
static u8 floodfill_visited[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

static inline void addFace(int x, int y, int z, Direction dir, BlockId block, u8 metadata, int ao, bool transparent) {
	if (x >= 0 && y >= 0 && z >= 0 && x < CHUNK_SIZE && y < CHUNK_SIZE && z < CHUNK_SIZE) {
		faceBuffer[currentFace++] = (Face){ x, y, z, dir, block, ao, metadata, transparent };
		transparentFaces += transparent;
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

		for (int i = 0; i < 6; i++) {
			const s8* offset = DirectionToOffset[i];
			int x			 = item.x + offset[0];
			int y			 = item.y + offset[1];
			int z			 = item.z + offset[2];
			if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE) {
				exitPoints[i] = true;
			} else {
				if (!(BLOCKS[cluster->blocks[x][y][z]]->opaque) && !(floodfill_visited[x][y][z] & 1)) {
					floodfill_visited[x][y][z] |= 1;
					vec_push(&floodfill_queue, ((QueueElement){ x, y, z }));
				}

				if ((cluster->blocks[item.x][item.y][item.z] == BLOCK_AIR || BLOCKS[cluster->blocks[x][y][z]]->opaque) &&
					cluster->blocks[x][y][z] != BLOCK_AIR) {
					u8 meta			 = cluster->metadataLight[x][z][z] & 0xf;
					bool transparent = Block_GetMaterial(BLOCKS[cluster->blocks[x][z][z]])->transculent;

					addFace(x, y, z, DirectionOpposite[i], cluster->blocks[x][y][z], meta, 0, transparent);
				}
			}
		}
	}
	u16 seeThrough = 0;
	for (int i = 0; i < 6; i++)
		if (exitPoints[i])
			for (int j = 0; j < 6; j++)
				if (i != j && exitPoints[j])
					seeThrough |= ChunkSeeThrough(i, j);
	return seeThrough;
}

void PolyGen_GeneratePolygons(WorkerItem item, void* this) {
	for (int i = 0; i < CLUSTER_PER_CHUNK; i++) {
		Cluster* cluster = &item.chunk->clusters[i];

		if (cluster->revision == cluster->vboRevision && !cluster->forceVBOUpdate)
			continue;

		cluster->vboRevision	= cluster->revision;
		cluster->forceVBOUpdate = false;

		currentFace		 = 0;
		transparentFaces = 0;

		u16 seeThrough = 0;

		memset(floodfill_visited, 0, sizeof(floodfill_visited));
		for (int x = 0; x < CHUNK_SIZE; x += CHUNK_SIZE - 1) {
			Direction xDir = !x ? Direction_West : Direction_East;

			for (int z = 0; z < CHUNK_SIZE; z++) {
				Direction zDir = z == 0 ? Direction_North : z == CHUNK_SIZE - 1 ? Direction_South : Direction_None;

				for (int y = 0; y < CHUNK_SIZE; y++) {
					Direction yDir = y == 0 ? Direction_Bottom : y == CHUNK_SIZE - 1 ? Direction_Top : Direction_None;

					Block* block = BLOCKS[cluster->blocks[x][y][z]];
					if (!block->opaque)
						seeThrough |= floodFill(cluster, x, y, z, xDir, yDir, zDir);

					Block* blockNeighbor = BLOCKS[fastBlockFetch(item.chunk, cluster, x + (!x ? -1 : 1), y, z)];

					if (!blockNeighbor->opaque && cluster->blocks[x][y][z] != BLOCK_AIR) {
						addFace(x, y, z, xDir, cluster->blocks[x][y][z], cluster->metadataLight[x][y][z] & 0xf, 0,
								Block_GetMaterial(block)->transculent);
					}
				}
			}
		}
		for (int y = 0; y < CHUNK_SIZE; y += CHUNK_SIZE - 1) {
			Direction yDir = !y ? Direction_Bottom : Direction_Top;

			for (int x = 0; x < CHUNK_SIZE; x++) {
				Direction xDir = x == 0 ? Direction_West : x == CHUNK_SIZE - 1 ? Direction_East : Direction_None;

				for (int z = 0; z < CHUNK_SIZE; z++) {
					Direction zDir = z == 0 ? Direction_South : x == CHUNK_SIZE - 1 ? Direction_North : Direction_None;

					Block* block = BLOCKS[cluster->blocks[x][y][z]];
					if (!block->opaque)
						seeThrough |= floodFill(cluster, x, y, z, xDir, yDir, zDir);

					Block* blockNeighbor = BLOCKS[fastBlockFetch(item.chunk, cluster, x, y + (!y ? -1 : 1), z)];

					if (!blockNeighbor->opaque && cluster->blocks[x][y][z] != BLOCK_AIR) {
						addFace(x, y, z, yDir, cluster->blocks[x][y][z], cluster->metadataLight[x][y][z] & 0xf, 0,
								Block_GetMaterial(block)->transculent);
					}
				}
			}
		}
		for (int z = 0; z < CHUNK_SIZE; z += CHUNK_SIZE - 1) {
			Direction zDir = !z ? Direction_North : Direction_South;

			for (int x = 0; x < CHUNK_SIZE; x++) {
				Direction xDir = x == 0 ? Direction_West : x == CHUNK_SIZE - 1 ? Direction_East : Direction_None;

				for (int y = 0; y < CHUNK_SIZE; y++) {
					Direction yDir = y == 0 ? Direction_Bottom : y == CHUNK_SIZE ? Direction_Top : Direction_None;

					Block* block = BLOCKS[cluster->blocks[x][y][z]];
					if (!block->opaque)
						seeThrough |= floodFill(cluster, x, y, z, xDir, yDir, zDir);

					Block* blockNeighbor = BLOCKS[fastBlockFetch(item.chunk, cluster, x, y, z + (!z ? -1 : 1))];

					if (!blockNeighbor->opaque && cluster->blocks[x][y][z] != BLOCK_AIR) {
						addFace(x, y, z, zDir, cluster->blocks[x][y][z], cluster->metadataLight[x][y][z] & 0xf, 0,
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

		int transparentVertices = transparentFaces * 6;
		int opaqueVertices		= (currentFace * 6) - transparentVertices;
		VBOUpdate update;

		if (currentFace) {
			VBO_Block memBlock;
			if (opaqueVertices > 0)
				memBlock = VBO_Alloc(opaqueVertices * sizeof(WorldVertex));
			VBO_Block transparentMem;
			if (transparentFaces > 0)
				transparentMem = VBO_Alloc(transparentVertices * sizeof(WorldVertex));

			WorldVertex* opaqueData		 = memBlock.memory;
			WorldVertex* transparentData = transparentMem.memory;
			for (int j = 0; j < currentFace; j++) {
				Face face = faceBuffer[j];

				int offsetX = face.x + item.chunk->x * CHUNK_SIZE;
				int offsetZ = face.z + item.chunk->z * CHUNK_SIZE;
				int offsetY = face.y + i * CHUNK_SIZE;

				s16 iconUV[2];
				Block_GetBlockTexture(BLOCKS[face.block], face.direction, face.x, face.y, face.z, face.metadata, iconUV);

				WorldVertex* data = face.transparent ? transparentData : opaqueData;
				memcpy(data, &block_sides_lut[face.direction * 6], sizeof(WorldVertex) * 6);

				u8 color[3];
				Block_GetBlockColor(BLOCKS[face.block], face.direction, face.x, face.y, face.z, face.metadata, color);

#define oneDivIconsPerRow (32768 / 8)
#define halfTexel (6)

				for (int k = 0; k < 6; k++) {
					data[k].pos[0] += offsetX;
					data[k].pos[1] += offsetY;
					data[k].pos[2] += offsetZ;
					data[k].uv0[0] = (data[k].uv0[0] == 1 ? (oneDivIconsPerRow - 1) : 1) + iconUV[0];
					data[k].uv0[1] = (data[k].uv0[1] == 1 ? (oneDivIconsPerRow - 1) : 1) + iconUV[1];

					data[k].rgb[0] = color[0];
					data[k].rgb[1] = color[1];
					data[k].rgb[2] = color[2];
				}
				if (face.transparent)
					transparentData += 6;
				else
					opaqueData += 6;
			}

			update.vbo			  = memBlock;
			update.transparentVBO = transparentMem;
		}

		update.x				   = item.chunk->x;
		update.y				   = i;
		update.z				   = item.chunk->z;
		update.vertices			   = opaqueVertices;
		update.delay			   = 0;
		update.seeThrough		   = seeThrough;
		update.transparentVertices = transparentVertices;

		LightLock_Lock(&updateLock);
		vec_push(&vboUpdates, update);
		LightLock_Unlock(&updateLock);
	}
	item.chunk->displayRevision = item.chunk->revision;
	item.chunk->forceVBOUpdate	= false;
}
