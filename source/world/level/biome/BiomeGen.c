#include "world/level/biome/BiomeGen.h"

#include <malloc.h>
#include <string.h>

#include "world/GrassColors.h"

static void placeholder() {
}

BiomeGenVtable vtable_default = { .placeholder = placeholder };

BiomeGen* BiomeGen_Init() {
	BiomeGen* b = (BiomeGen*)malloc(sizeof(BiomeGen));

	b->vptr = (BiomeGenVtable*)malloc(sizeof(BiomeGenVtable));
	if (b->vptr)
		memcpy(b->vptr, &vtable_default, sizeof(BiomeGenVtable));

	b->topBlock	   = BLOCK_GRASS;
	b->fillerBlock = BLOCK_DIRT;
	b->snows	   = true;
	b->rains	   = true;
	b->color	   = 5169201;
	b->heightMin   = 10;
	b->heightMax   = 30;
	b->temperature = 5;
	b->rainfall	   = 5;
	b->waterColor  = 16777215;

	return b;
}
float BiomeGen_GetFloatRainfall(BiomeGen* b) {
	return (float)(b->rainfall / 10.f);
}
float BiomeGen_GetFloatTemperature(BiomeGen* b) {
	return (float)(b->temperature / 10.f);
}
u32 BiomeGen_GetGrassColor(BiomeGen* b) {
	return GrassColors_Get(BiomeGen_GetFloatRainfall(b), BiomeGen_GetFloatTemperature(b));
}