#include "world/level/block/material/Material.h"

const Material MATERIALS[MATERIAL_COUNT] = {
	[MATERIAL_AIR]			 = { .color = COLOR_AIR, .replaceable = false },
	[MATERIAL_EARTH]		 = { .color = COLOR_DIRT, .solid = true, .opaque = true },
	[MATERIAL_ORGANIC]		 = { .color = COLOR_GRASS, .solid = true, .opaque = true },
	[MATERIAL_LEAVES]		 = { .color = COLOR_FOLIAGE, .solid = true, .flammable = true },
	[MATERIAL_WOOD]			 = { .color = COLOR_WOOD, .solid = true, .opaque = true, .flammable = true },
	[MATERIAL_WOOL]			 = { .color = COLOR_WOOL, .solid = true, .opaque = true, .flammable = true },
	[MATERIAL_ROCK]			 = { .color = COLOR_STONE, .solid = true, .opaque = true },
	[MATERIAL_CAKE]			 = { .color = COLOR_AIR, .solid = true, .opaque = true },
	[MATERIAL_MISCELLANEOUS] = { .color = COLOR_AIR },
};