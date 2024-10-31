#include "world/level/block/Block.h"

#include "client/Crash.h"
#include "core/Direction.h"
#include "util/StringUtils.h"
#include "world/World.h"

#include "stdio.h"

static bool renderAsNormalBlock() {
	return true;
}
static u8 getRenderType() {
	return 0;
}
static bool isOpaqueCube() {
	return true;
}
static void registerIcons(Block* block) {
	const char* path = String_ParseTexturePath("block", block->name);
	block->icon		 = Texture_MapAdd(path);
}
static int getBlockColor(Block* b, Direction dir, u8 meta) {
	return 0xFFFFFFFF;
}
static u16 getBlockTexture(Block* block, Direction dir, int x, int y, int z, u8 metadata) {
	return block->icon;
}

static BlockVtable vtable_default = {
	.renderAsNormalBlock = renderAsNormalBlock,
	.getRenderType		 = getRenderType,
	.isOpaqueCube		 = isOpaqueCube,
	.registerIcons		 = registerIcons,
	.getBlockColor		 = getBlockColor,
	.getBlockTexture	 = getBlockTexture,
};

static const Box boxDefault = { { { 0.0F, 0.0F, 0.0F } }, { { 1.0F, 1.0F, 1.0F } } };

Block* Block_Init(const char* name, BlockId id, float resistance, float hardness, MaterialType material) {
	return Block_InitWithBounds(name, id, resistance, hardness, material, boxDefault);
}

Block* Block_InitWithBounds(const char* name, BlockId id, float resistance, float hardness, MaterialType material, Box bounds) {
	Block* b = (Block*)malloc(sizeof(Block));

	b->vptr = (BlockVtable*)malloc(sizeof(BlockVtable));
	if (b->vptr)
		memcpy(b->vptr, &vtable_default, sizeof(BlockVtable));

	Block_SetResistance(b, resistance);
	Block_SetHardness(b, hardness);
	b->id		= id;
	b->material = material;
	memcpy(&b->bounds, &bounds, sizeof(Box));
	strcpy(b->name, name);

	return b;
}

void Block_SetResistance(Block* b, float v) {
	b->resistance = v * 3.f;
}
void Block_SetHardness(Block* b, float v) {
	b->hardness = v;

	if (b->resistance < v * 5.f)
		b->resistance = v * 5.f;
}
void Block_SetBounds(Block* b, float3 from, float3 to) {
	b->bounds.min = from;
	b->bounds.max = to;
}

// if is smaller than block or sight hidden by other block
bool Block_ShouldSideBeRendered(Block* b, int x, int y, int z, Direction dir) {
	if (b->vptr->isOpaqueCube) {
		return !World_IsBlockOpaqueCube(x, y, z);
	} else {
		return dir == Direction_Bottom && b->bounds.min.y > 0.f
				   ? true
				   : (dir == Direction_Top && b->bounds.max.y < 1.f
						  ? true
						  : (dir == Direction_North && b->bounds.min.z > 0.f
								 ? true
								 : (dir == Direction_South && b->bounds.max.z < 1.f
										? true
										: (dir == Direction_West && b->bounds.min.x > 0.f
											   ? true
											   : (dir == Direction_East && b->bounds.max.x < 1.f ? true
																								 : !World_IsBlockOpaqueCube(x, y, z))))));
	}
}

const Material* Block_GetMaterial(Block* block) {
	return &MATERIALS[block->material];
}