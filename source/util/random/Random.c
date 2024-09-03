#include "util/random/Random.h"

#include <stdlib.h>

#include "util/random/RandomSeed.h"

Random* Random_Init(u32 max) {
	return Random_InitWithSeed(RandomSeed_Gen(), max);
}
Random* Random_InitWithSeed(u64 seed, u32 max) {
	Random* rand = malloc(sizeof(Random));

	rand->max	= max;
	rand->state = seed;

	return rand;
}

u32 Random_Next(Random* rnd) {
	rnd->state = rnd->state * 0x5DEECE66DL + 0xB;

	return (u32)(rnd->state >> (48 - 31)) % (rnd->max + 1);
}