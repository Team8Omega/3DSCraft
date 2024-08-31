#include "util/random/WeightedRandom.h"

#include <stdlib.h>

static RandomSeed seed = 0;

WeightedRandom* WeightedRandom_Init(size_t numEntry, float entries[]) {
	if (!seed)
		seed = RandomSeed_Gen();

	WeightedRandom* ran = malloc(sizeof(WeightedRandom));

	ran->length = numEntry;
	ran->vals	= malloc(sizeof(u32) * numEntry);

	float totalSize = 0.f;
	for (size_t i = 0; i < numEntry; ++i) {
		totalSize += entries[i];
	}

	for (size_t i = 0; i < numEntry; ++i) {
		float weight = (float)(entries[i] / totalSize);
		u32 val		 = weight * 100;
		ran->vals[i] = val;
	}

	return ran;
}

u32 WeightedRandom_GetRandom(WeightedRandom* ran) {
	srand(seed);

	u32 result = rand() % 100;

	for (size_t i = 0; i < ran->length; ++i) {
		result -= ran->vals[i];

		if (result < 0) {
			return (u32)i;
		}
	}
}