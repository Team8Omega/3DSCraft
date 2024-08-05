#pragma once

#include "BiomeGen.h"

typedef enum
{
	BIOME_PLAINS,
	BIOME_COUNT
} BiomeGenType;

extern BiomeGen* BIOMEGENTYPES[BIOME_COUNT];

void BiomeGenType_Init();