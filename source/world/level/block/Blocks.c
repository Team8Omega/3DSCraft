#include "world/level/block/Blocks.h"

#include "client/renderer/texture/TextureMap.h"
#include "util/math/VecMath.h"
#include "world/level/block/Block.h"
#include "world/level/block/material/Material.h"
#include "world/phys/Collision.h"

Block* BLOCKS[BLOCK_COUNT] = { NULL };

Texture_Map gTexMapBlock;

const Box boxDefault = { { { 0.0F, 0.0F, 0.0F } }, { { 1.0F, 1.0F, 1.0F } } };

void Blocks_Init() {
	Texture_MapAdd("romfs:/error.png");

	BLOCKS[BLOCK_AIR]	= Block_Init("air", 0, BLOCK_AIR, 0.f, 0.f, MATERIAL_AIR, boxDefault);
	BLOCKS[BLOCK_STONE] = Block_Init("stone", "stone", BLOCK_STONE, 1.5f, 10.f, MATERIAL_ROCK, boxDefault);
	// BLOCKS[BLOCK_GRASS] = Block_Init("grass", "grass", BLOCK_GRASS, 0.f, 0.6f, MATERIAL_GRASS, boxDefault);

	Texture_MapInit(&gTexMapBlock);
}