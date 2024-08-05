#include "world/level/biome/BiomeGenPlains.h"

#include "world/level/biome/BiomeGenType.h"

BiomeGen* BiomeGenPlains_Init() {
	BiomeGen* biome	   = BiomeGen_Init();
	biome->biomeName   = "Plains";
	biome->id		   = BIOME_PLAINS;
	biome->color	   = 9286496;
	biome->rainfall	   = 8;
	biome->temperature = 4;

	return biome;
}