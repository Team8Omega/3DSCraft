#include "world/level/block/BlockGrass.h"

#include "util/StringUtils.h"
#include "world/World.h"

u16 iconGrassTop, iconSnowSide, iconGrassOverlay;

static void registerIcons(Block* block) {
	const char* path;

	path			 = String_ParseTexturePrefix("block", block->name, "_side");
	block->icon		 = Texture_MapAdd(path);
	path			 = String_ParseTexturePrefix("block", block->name, "_top");
	iconGrassTop	 = Texture_MapAdd(path);
	path			 = String_ParseTexturePrefix("block", block->name, "_side_snow");
	iconSnowSide	 = Texture_MapAdd(path);
	path			 = String_ParseTexturePrefix("block", block->name, "_side_overlay");
	iconGrassOverlay = Texture_MapAdd(path);
}
static int getBlockColor(Block* b, Direction dir, u8 meta) {
	return dir == Direction_Top ? MATERIALS[b->material].color : 0xFFFFFFFF;
}
static u16 getBlockTexture(Block* block, Direction dir, int x, int y, int z, u8 metadata) {
	switch (dir) {
		case Direction_Top:
			return iconGrassTop;
			break;

		case Direction_Bottom:
			return Block_GetIcon(BLOCKS[BLOCK_STONE], Direction_North, 0);

		default:
			const Material* mat = World_GetMaterial(x, y + 1, z);
			return mat->id != MATERIAL_SNOW && mat->id != MATERIAL_CRAFTED_SNOW ? block->icon : iconSnowSide;
	}
}

Block* BlockGrass_Init(BlockId id) {
	Block* b = Block_Init("grass_block", id, 0.f, 0.6f, MATERIAL_GRASS);

	b->vptr->registerIcons	 = registerIcons;
	b->vptr->getBlockColor	 = getBlockColor;
	b->vptr->getBlockTexture = getBlockTexture;

	return b;
}