#pragma once

#include "world/level/material/MapColor.h"

typedef struct {
	u16 id;
	bool flammable;
	bool replaceable;
	bool solid;
	bool requiresTool;
	bool cantBlockGrass;
	bool transculent;
} Material;

typedef u8 MaterialType;
enum
{
	MATERIAL_AIR,
	MATERIAL_GRASS,
	MATERIAL_DIRT,
	MATERIAL_WOOD,
	MATERIAL_STONE,
	MATERIAL_LEAVES,
	MATERIAL_WOOL,
	MATERIAL_CAKE,
	MATERIAL_SNOW,
	MATERIAL_CRAFTED_SNOW,
	MATERIAL_MISCELLANEOUS,
	MATERIAL_COUNT
};

extern const Material MATERIALS[MATERIAL_COUNT];