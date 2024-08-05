#include "world/level/biome/BiomeGenType.h"

#include "world/level/biome/BiomeGenPlains.h"

BiomeGen* BIOMEGENTYPES[BIOME_COUNT] = { NULL };

void BiomeGenType_Init() {
	BIOMEGENTYPES[BIOME_PLAINS] = BiomeGenPlains_Init();
}