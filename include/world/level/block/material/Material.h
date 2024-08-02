#pragma once

#include "world/level/block/material/MaterialColor.h"

typedef struct {
	MaterialColor color;
	bool flammable;
	bool opaque;
	bool replaceable;
	bool solid;
} Material;

typedef enum
{
	MATERIAL_AIR,
	MATERIAL_EARTH,
	MATERIAL_ORGANIC,
	MATERIAL_LEAVES,
	MATERIAL_WOOD,
	MATERIAL_WOOL,
	MATERIAL_ROCK,
	MATERIAL_CAKE,
	MATERIAL_MISCELLANEOUS,
	MATERIAL_COUNT
} MaterialType;

extern const Material MATERIALS[MATERIAL_COUNT];