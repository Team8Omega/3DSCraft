#pragma once

#include "world/level/block/material/MaterialColor.h"

typedef struct {
	u16 id;
	MaterialColor color;
	bool flammable;
	bool opaque;
	bool replaceable;
	bool solid;
	bool requiresTool;
	bool cantBlockGrass;
	bool transculent;
} Material;

typedef enum
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
} MaterialType;

extern const Material MATERIALS[MATERIAL_COUNT];