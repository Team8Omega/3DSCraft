#pragma once

#include "world/level/block/material/MaterialColor.h"

typedef struct {
	MaterialColor color;
	bool flammable;
	bool opaque;
	bool replaceable;
	bool solid;
	bool requiresTool;
} Material;

typedef enum
{
	MATERIAL_AIR,
	MATERIAL_GRASS,
	MATERIAL_GROUND,
	MATERIAL_WOOD,
	MATERIAL_ROCK,
	MATERIAL_LEAVES,
	MATERIAL_WOOL,
	MATERIAL_CAKE,
	MATERIAL_MISCELLANEOUS,
	MATERIAL_COUNT
} MaterialType;

extern const Material MATERIALS[MATERIAL_COUNT];