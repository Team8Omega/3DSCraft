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
	size_t numVariants;
	WeightedRandom* weightedRandom;
} BlockState;

extern BlockState BLOCKSTATES[BLOCK_COUNT];

void BlockStates_Decompile();