#include "world/level/levelgen/SuperFlatGen.h"

#include "util/math/NumberUtils.h"

void SuperFlatGen_Init(SuperFlatGen* gen) {
}

void SuperFlatGen_Generate(WorkerItem item, void* this) {
	for (int y = 0; y < 17; y++) {
		BlockId block = BLOCK_AIR;
		switch (y) {  // TODO: Mit einem Lookup Table ersetzen, Superflach Gen konfigurierbar machen
			case 0:
				block = BLOCK_STONE;  // Block_Bedrock;
				break;
			case 1 ... 10:
				block = BLOCK_STONE;
				break;
			case 11 ... 15:
				block = BLOCK_DIRT;
				break;
			case 16:
				block = BLOCK_GRASS;
				break;
			default:
				block = BLOCK_AIR;
				break;
		}
		for (int x = 0; x < CHUNK_SIZE; x++)
			for (int z = 0; z < CHUNK_SIZE; z++)
				Chunk_SetBlock(item.chunk, x, y, z, block);
	}
}
