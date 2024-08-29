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

void regist(BlockId id, const char* name, float resistance, float hardness, MaterialType mat, MapColor col) {
	BLOCKS[id] = Block_Init(name, id, resistance, hardness, mat, col);
}

void Blocks_Init() {
	Texture_MapAdd("romfs:/error.png");

	regist(BLOCK_AIR, "air", 0.f, 0.f, MATERIAL_AIR, MAPCOLOR_NONE);
	Block_SetNotOpaque(BLOCKS[BLOCK_AIR]);

	regist(BLOCK_STONE, "stone", 1.5f, 6.0, MATERIAL_STONE, MAPCOLOR_STONE);
	regist(BLOCK_GRANITE, "granite", 1.5f, 6.0, MATERIAL_STONE, MAPCOLOR_DIRT);
	regist(BLOCK_POLISHED_GRANITE, "polished_granite", .5f, 6.0, MATERIAL_STONE, MAPCOLOR_DIRT);
	regist(BLOCK_DIORITE, "diorite", 1.5f, 6.0, MATERIAL_STONE, MAPCOLOR_QUARTZ);
	regist(BLOCK_POLISHED_DIORITE, "polished_diorite", 1.5f, 6.0, MATERIAL_STONE, MAPCOLOR_QUARTZ);
	regist(BLOCK_ANDESITE, "andesite", 1.5f, 6.0, MATERIAL_STONE, MAPCOLOR_STONE);
	regist(BLOCK_POLISHED_ANDESITE, "polished_andesite", 1.5f, 6.0, MATERIAL_STONE, MAPCOLOR_STONE);
	BLOCKS[BLOCK_GRASS] = BlockGrass_Init(BLOCK_GRASS);
	regist(BLOCK_DIRT, "dirt", 0.5f, 0.5f, MATERIAL_DIRT, MAPCOLOR_DIRT);
	regist(BLOCK_COARSE_DIRT, "coarse_dirt", 0.5f, 0.5f, MATERIAL_DIRT, MAPCOLOR_DIRT);
	regist(BLOCK_PODZOL, "podzol", 0.5f, 0.5f, MATERIAL_DIRT, MAPCOLOR_DIRT);
	regist(BLOCK_COBBLESTONE, "cobblestone", 2.0f, 6.0f, MATERIAL_STONE, MAPCOLOR_STONE);
	regist(BLOCK_OAK_PLANKS, "oak_planks", 2.0f, 6.0f, MATERIAL_WOOD, MAPCOLOR_WOOD);
	regist(BLOCK_SPRUCE_PLANKS, "spruce_planks", 2.0f, 6.0f, MATERIAL_WOOD, MAPCOLOR_WOOD);
	regist(BLOCK_BIRCH_PLANKS, "birch_planks", 2.0f, 6.0f, MATERIAL_WOOD, MAPCOLOR_WOOD);
	regist(BLOCK_JUNGLE_PLANKS, "jungle_planks", 2.0f, 6.0f, MATERIAL_WOOD, MAPCOLOR_WOOD);
	regist(BLOCK_ACACIA_PLANKS, "acacia_planks", 2.0f, 6.0f, MATERIAL_WOOD, MAPCOLOR_WOOD);
	regist(BLOCK_DARK_OAK_PLANKS, "dark_oak_planks", 2.0f, 6.0f, MATERIAL_WOOD, MAPCOLOR_WOOD);
	regist(BLOCK_BEDROCK, "bedrock", -0.1f, 3600000.0F, MATERIAL_STONE, MAPCOLOR_STONE);

	for (BlockId i = 0; i < BLOCK_COUNT; ++i) {
		if (BLOCKS[i] == NULL)
			Crash("ERROR: Block/s missing in initializer loop, please fix. internal error.");

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
}