#pragma once

#include "BiomeGen.h"

typedef u8 BiomeGenType;
enum
{
	BIOME_PLAINS,
	BIOME_COUNT
};

extern BiomeGen* BIOMEGENTYPES[BIOME_COUNT];

void BiomeGenType_Init();