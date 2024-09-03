#pragma once

typedef struct {
	u64 state;
	u32 max;
} Random;

Random* Random_InitWithSeed(u64 seed, u32 max);
Random* Random_Init(u32 max);

u32 Random_Next(Random* rnd);