#include "util/random/WeightedRandom.h"

#include <stdlib.h>

#include "client/Crash.h"
#include "util/random/Random.h"

static Random* gen = NULL;

WeightedRandom* WeightedRandom_Init(size_t numEntry, float entries[]) {
	if (!gen)
		gen = Random_Init(100);

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
	u32 result = Random_Next(gen);

	for (u32 i = 0; i < ran->length; ++i) {
		if (ran->vals[i] > result)
			return i;

		result -= ran->vals[i];
	}

	return 0;
}