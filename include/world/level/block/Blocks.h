#pragma once

typedef struct Block Block;

typedef u8 BlockId;
enum
{
	BLOCK_AIR,
	BLOCK_STONE,
	BLOCK_GRASS,
	BLOCK_DIRT,
	BLOCK_GRANITE,
	BLOCK_POLISHED_GRANITE,
	BLOCK_COUNT,
};

extern Block* BLOCKS[BLOCK_COUNT];

void Blocks_Init();