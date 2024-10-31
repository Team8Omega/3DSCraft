#pragma once

typedef struct Block Block;

typedef enum
{
	BLOCK_AIR,
	BLOCK_STONE,
	BLOCK_GRASS,
	BLOCK_COUNT
} BlockId;

extern Block* BLOCKS[BLOCK_COUNT];

void Blocks_Init();