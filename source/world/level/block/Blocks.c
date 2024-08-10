#include "world/level/block/Blocks.h"

#include "client/Crash.h"
#include "client/renderer/texture/TextureMap.h"
#include "util/math/VecMath.h"
#include "world/level/material/Material.h"
#include "world/phys/Collision.h"

#include "world/level/block/Block.h"
#include "world/level/block/BlockGrass.h"

Block* BLOCKS[BLOCK_COUNT] = { NULL };

Texture_Map gTexMapBlock;

void Blocks_Init() {
	Texture_MapAdd("romfs:/error.png");

	BLOCKS[BLOCK_AIR]			   = Block_Init("air", BLOCK_AIR, 0.f, 0.f, MATERIAL_AIR, MAPCOLOR_AIR);
	BLOCKS[BLOCK_STONE]			   = Block_Init("stone", BLOCK_STONE, 1.5f, 6.0, MATERIAL_STONE, MAPCOLOR_STONE);
	BLOCKS[BLOCK_GRASS]			   = BlockGrass_Init(BLOCK_GRASS);
	BLOCKS[BLOCK_DIRT]			   = Block_Init("dirt", BLOCK_DIRT, 0.5f, 0.5f, MATERIAL_DIRT, MAPCOLOR_DIRT);
	BLOCKS[BLOCK_GRANITE]		   = Block_Init("granite", BLOCK_STONE, 1.5f, 6.0, MATERIAL_DIRT, MAPCOLOR_DIRT);
	BLOCKS[BLOCK_POLISHED_GRANITE] = Block_Init("polished_granite", BLOCK_STONE, 1.5f, 6.0, MATERIAL_DIRT, MAPCOLOR_DIRT);

	Block_SetNotOpaque(BLOCKS[BLOCK_AIR]);

	for (BlockId i = 0; i < BLOCK_COUNT; ++i) {
		if (BLOCKS[i] == NULL)
			Crash("ERROR: Block/s missing in initializer loop, please fix. internal error.");

		BLOCKS[i]->vptr->registerIcons(BLOCKS[i]);

		{
			bool sw = false;

			if (BLOCKS[i]->renderType == 10)
				sw = true;
			// else if(i == BLOCK_FARMLAND)
			// 	sw=true;
			else if (!MATERIALS[BLOCKS[i]->material].cantBlockGrass)
				sw = true;
			else if (BLOCKS[i]->lightness == 0)
				sw = true;

			BLOCKS[i]->useNeighborBrightness = sw;
		}
	}

	Texture_MapInit(&gTexMapBlock);
}