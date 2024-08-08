#include "world/level/material/Material.h"

// clang-format off
const Material MATERIALS[MATERIAL_COUNT] = {
	[MATERIAL_AIR]			 = { .id = MATERIAL_AIR, .cantBlockGrass = true, .transculent=true },
	[MATERIAL_GRASS]		 = { .id = MATERIAL_GRASS, .solid = true },
	[MATERIAL_DIRT]			 = { .id = MATERIAL_DIRT, .solid = true },
	[MATERIAL_WOOD]			 = { .id = MATERIAL_WOOD, .solid = true, .flammable = true },
	[MATERIAL_STONE]		 = { .id = MATERIAL_STONE, .solid = true, .requiresTool = true },
	[MATERIAL_LEAVES]		 = { .id = MATERIAL_LEAVES, .solid = true, .flammable = true },
	[MATERIAL_WOOL]			 = { .id = MATERIAL_WOOL,.solid = true, .flammable = true },
	[MATERIAL_CAKE]			 = { .id = MATERIAL_CAKE, .solid = true },
	[MATERIAL_SNOW]			 = { .id = MATERIAL_SNOW, .cantBlockGrass = true, .replaceable = true, .requiresTool = true, .transculent = true},
	[MATERIAL_CRAFTED_SNOW]	 = { .id = MATERIAL_CRAFTED_SNOW,  .solid = true, .replaceable = true, .requiresTool = true},
	[MATERIAL_MISCELLANEOUS] = { .id = MATERIAL_MISCELLANEOUS},
};
// clang-format on