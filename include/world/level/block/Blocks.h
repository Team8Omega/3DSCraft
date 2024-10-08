#pragma once

typedef struct Block Block;

typedef u8 BlockId;
enum {
	BLOCK_AIR = 0,
	BLOCK_STONE,
	BLOCK_GRANITE,
	BLOCK_POLISHED_GRANITE,
	BLOCK_DIORITE,
	BLOCK_POLISHED_DIORITE,
	BLOCK_ANDESITE,
	BLOCK_POLISHED_ANDESITE,
	BLOCK_GRASS,
	BLOCK_DIRT,
	/*BLOCK_COARSE_DIRT,
	BLOCK_PODZOL,
	BLOCK_COBBLESTONE,
	BLOCK_OAK_PLANKS,
	BLOCK_SPRUCE_PLANKS,
	BLOCK_BIRCH_PLANKS,
	BLOCK_JUNGLE_PLANKS,
	BLOCK_ACACIA_PLANKS,
	BLOCK_DARK_OAK_PLANKS,
	*/
	BLOCK_BEDROCK,
	BLOCK_COUNT,
};

extern Block* BLOCKS[BLOCK_COUNT];

void Blocks_Init();