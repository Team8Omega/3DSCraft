#include "world/level/levelgen/SmeaGen.h"

#include <sino/sino.h>

void SmeaGen_Init(SmeaGen* gen) {
}
// based off https://github.com/smealum/3dscraft/blob/master/source/generation.c
void SmeaGen_Generate(WorkerItem item, void* this) {
	for (int x = 0; x < CHUNK_SIZE; x++) {
		for (int z = 0; z < CHUNK_SIZE; z++) {
			float px = (float)(x + item.chunk->x * CHUNK_SIZE);
			float pz = (float)(z + item.chunk->z * CHUNK_SIZE);

			const int smeasClusterSize = 8;
			const int smeasChunkHeight = 16;
			int height = 16; /*(int)(sino_2d((px) / (smeasClusterSize * 4), (pz) / (smeasClusterSize * 4)) * smeasClusterSize) +
		  (smeasChunkHeight * smeasClusterSize / 2);
*/
			Chunk_SetBlock(item.chunk, x, 0, z, BLOCK_BEDROCK);
			for (int y = 1; y < height - 3; ++y) {
				Chunk_SetBlock(item.chunk, x, y, z, BLOCK_STONE);
			}
			for (int y = height - 3; y < height; ++y) {
				Chunk_SetBlock(item.chunk, x, y, z, BLOCK_DIRT);
			}
			Chunk_SetBlock(item.chunk, x, height, z, BLOCK_GRASS);
		}
	}
}
