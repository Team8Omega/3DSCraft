#include "world/level/block/Blocks.h"

#include "client/renderer/texture/TextureMap.h"
#include "util/math/VecMath.h"
#include "world/level/block/material/Material.h"
#include "world/phys/Collision.h"

#include "world/level/block/Block.h"
#include "world/level/block/BlockGrass.h"

Block* BLOCKS[BLOCK_COUNT] = { NULL };

Texture_Map gTexMapBlock;

void Blocks_Init() {
	Texture_MapAdd("romfs:/error.png");

	BLOCKS[BLOCK_AIR]	= Block_Init("air", BLOCK_AIR, 0.f, 0.f, MATERIAL_AIR);
	BLOCKS[BLOCK_STONE] = Block_Init("stone", BLOCK_STONE, 1.5f, 10.f, MATERIAL_STONE);
	BLOCKS[BLOCK_GRASS] = BlockGrass_Init(BLOCK_GRASS);

	for (BlockId i = 1; i < BLOCK_COUNT; ++i) {
		BLOCKS[i]->vptr->registerIcons(BLOCKS[i]);
	}

	Texture_MapInit(&gTexMapBlock);
}