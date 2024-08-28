#pragma once

#include "RandomSeed.h"

typedef struct {
	u32* vals;
	size_t num;
	RandomSeed seed;
} WeightedRandom;

WeightedRandom* WeightedRandom_Init(size_t numEntry, float entries[]);
u32 WeightedRandom_GetRandom(WeightedRandom* weightedRand);