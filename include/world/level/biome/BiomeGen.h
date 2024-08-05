#pragma once

#include "world/level/block/Blocks.h"

typedef struct BiomeGen BiomeGen;

typedef struct {
	void (*placeholder)();
} BiomeGenVtable;

struct BiomeGen {
	BiomeGenVtable* vptr;	// function calls for each biome.
	u8 id;					// id, matches enum.
	const char* biomeName;	// the name.
	u32 color;				// color, unknown yet
	u32 waterColor;			// multiplied with color, for water.
	BlockId topBlock;		// block expected on top
	BlockId fillerBlock;	// block expected under top
	u8 heightMin;			// minimum height, ranges from 0 to 100 as in 0.0f to 1.0f. default is 10
	u8 heightMax;			// maximum height, ranges from 0 to 100 as in 0.0f to 1.0f. default is 30
	s8 temperature;			// temperature, 0.1f == 1, -1.0f == -10.
	u8 rainfall;			// humidity, ranges from 0 to 10 as in 0.0f to 1.0f.
	bool snows;				// has snow or not
	bool rains;				// has rain or not
							// insert biome gens here
};

BiomeGen* BiomeGen_Init();
u32 BiomeGen_GetGrassColor(BiomeGen* biome);
float BiomeGen_GetIntRainfall(BiomeGen* biome);
float BiomeGen_GetIntTemperature(BiomeGen* biome);
float BiomeGen_GetFloatRainfall(BiomeGen* biome);
float BiomeGen_GetFloatTemperature(BiomeGen* biome);