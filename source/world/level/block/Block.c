#include "world/level/block/Block.h"

#include "core/Direction.h"
#include "util/StringUtils.h"
#include "util/math/NumberUtils.h"
#include "world/World.h"

#include "stdio.h"

static u8 getRenderType() {
	return 0;
}
static u32 getBlockColor(Block* b, Direction dir, int x, int y, int z, u8 meta) {
	return COLOR_WHITE;
}
static u32 getItemColor(Direction dir, u8 meta) {
	return COLOR_WHITE;
}

static BlockVtable vtable_default = {
	.getRenderType = getRenderType,
	.getBlockColor = getBlockColor,
	.getItemColor  = getItemColor,
};

static const Box boxDefault = { { { 0.0F, 0.0F, 0.0F } }, { { 1.0F, 1.0F, 1.0F } } };

Block* Block_Init(const char* name, BlockId id, float resistance, float hardness, MaterialType material, MapColor mapColor) {
	return Block_InitWithBounds(name, id, resistance, hardness, material, mapColor, boxDefault);
}

Block* Block_InitWithBounds(const char* name, BlockId id, float resistance, float hardness, MaterialType material, MapColor mapColor,
							Box bounds) {
	Block* b = (Block*)malloc(sizeof(Block));

	b->vptr = (BlockVtable*)malloc(sizeof(BlockVtable));
	if (b->vptr)
		memcpy(b->vptr, &vtable_default, sizeof(BlockVtable));

	Block_SetResistance(b, resistance);
	Block_SetHardness(b, hardness);
	Block_SetLightness(b, 0);
	b->name				 = strdup(name);
	b->id				 = id;
	b->material			 = material;
	b->renderType		 = 0;  // not sure anymore, but i think 0 = normal, 1 = tile entity idek
	b->mapColor			 = mapColor;
	b->opaque			 = true;
	b->solidBlock		 = true;
	b->hasRandomVariants = false;
	memcpy(&b->bounds, &bounds, sizeof(Box));

	return b;
}

void Block_SetResistance(Block* b, float v) {
	b->resistance = MAX(0.f, v);
}
void Block_SetNotOpaque(Block* b) {
	b->opaque = false;
}
void Block_SetHardness(Block* b, float v) {
	b->hardness = v;
}
void Block_SetBounds(Block* b, float3 from, float3 to) {
	b->bounds.min = from;
	b->bounds.max = to;
}
void Block_SetLightness(Block* b, u8 v) {
	b->lightness = v;
}
void Block_SetNotSolidBlock(Block* b) {
	b->solidBlock = false;
}

#define colR(c) ((c >> 16) & 0xff)
#define colG(c) (((c) >> 8) & 0xff)
#define colB(c) ((c) & 0xff)
#define COLOR_MUL 255 / 15

void Block_GetBlockColor(Block* b, Direction dir, int x, int y, int z, u8 meta, u8 out[]) {
	if (!b)
		return;

	u32 color = b->vptr->getBlockColor(b, dir, x, y, z, meta);

	u16 brightness = 15;  // 0 - 15. if implemented, check for max value (bitwise and)
	brightness *= COLOR_MUL;
	brightness--;

	out[0] = (colR(color) * brightness + brightness) / 255;
	out[1] = (colG(color) * brightness + brightness) / 255;
	out[2] = (colB(color) * brightness + brightness) / 255;

#define GRASS_ADD 63
	if (b->id == BLOCK_GRASS && dir == Direction_Top) {
		out[0] += GRASS_ADD;
		out[1] += GRASS_ADD;
		out[2] += GRASS_ADD;
	}
}

void Block_GetItemColor(Block* b, Direction dir, u8 meta, u8 out[]) {
	if (!b)
		return;

	u32 color = b->vptr->getItemColor(dir, meta);

	out[0] = colR(color);
	out[1] = colG(color);
	out[2] = colB(color);
}