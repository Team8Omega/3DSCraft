#include "world/level/block/material/Material.h"

// clang-format off
const Material MATERIALS[MATERIAL_COUNT] = {
	[MATERIAL_AIR]			 = { .id = MATERIAL_AIR, .color = COLOR_AIR, .cantBlockGrass = true },
	[MATERIAL_GRASS]		 = { .id = MATERIAL_GRASS, .color = COLOR_GRASS, .solid = true, .opaque = true },
	[MATERIAL_DIRT]			 = { .id = MATERIAL_DIRT, .color = COLOR_DIRT, .solid = true, .opaque = true },
	[MATERIAL_WOOD]			 = { .id = MATERIAL_WOOD, .color = COLOR_WOOD, .solid = true, .opaque = true, .flammable = true },
	[MATERIAL_STONE]		 = { .id = MATERIAL_STONE, .color = COLOR_STONE, .solid = true, .opaque = true, .requiresTool = true },
	[MATERIAL_LEAVES]		 = { .id = MATERIAL_LEAVES, .color = COLOR_FOLIAGE, .solid = true, .flammable = true },
	[MATERIAL_WOOL]			 = { .id = MATERIAL_WOOL, .color = COLOR_WOOL, .solid = true, .opaque = true, .flammable = true },
	[MATERIAL_CAKE]			 = { .id = MATERIAL_CAKE, .color = COLOR_AIR, .solid = true, .opaque = true },
	[MATERIAL_SNOW]			 = { .id = MATERIAL_SNOW, .color = COLOR_SNOW, .cantBlockGrass = true, .replaceable = true, .requiresTool = true, .transculent = true},
	[MATERIAL_CRAFTED_SNOW]	 = { .id = MATERIAL_CRAFTED_SNOW, .color = COLOR_SNOW, .solid = true, .opaque = true, .replaceable = true, .requiresTool = true},
	[MATERIAL_MISCELLANEOUS] = { .id = MATERIAL_MISCELLANEOUS, .color = COLOR_AIR },
};
// clang-format on