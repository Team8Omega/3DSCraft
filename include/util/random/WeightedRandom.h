#pragma once

#include "RandomSeed.h"

typedef struct {
	u32* vals;
	size_t length;
} WeightedRandom;

WeightedRandom* WeightedRandom_Init(size_t length, float entries[]);
u32 WeightedRandom_GetRandom(WeightedRandom* weightedRand);