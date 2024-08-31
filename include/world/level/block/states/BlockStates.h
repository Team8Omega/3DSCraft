#pragma once

#include "resources/model/BakedModel.h"
#include "util/random/WeightedRandom.h"
#include "world/level/block/Blocks.h"

typedef struct {
	u32 index;
	BakedModel* model;
} BlockStateVariant;

typedef struct {
	BlockStateVariant* variants;
	size_t variantNum;
	WeightedRandom* random;
} BlockState;

typedef struct {
	BlockState* states;
	size_t stateNum;
} BlockStateHolder;

extern BlockStateHolder BLOCKSTATES[BLOCK_COUNT];

void BlockStates_Decompile();

BlockStateVariant* BlockState_Get(BlockId blockId, u8 index);
