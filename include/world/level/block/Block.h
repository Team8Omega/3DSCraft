#pragma once

#include "client/Crash.h"
#include "client/renderer/texture/TextureMap.h"
#include "core/Direction.h"
#include "util/math/VecMath.h"
#include "world/phys/Collision.h"

#include "Blocks.h"
#include "material/Material.h"

typedef struct Block Block;

typedef struct {
	bool (*renderAsNormalBlock)();	// If to render as a normal block
	bool (*isOpaqueCube)();
	u8 (*getRenderType)();
	u8 (*getBlockColor)(Block *b, Direction dir, int x, int y, int z, u8 meta);
	u16 (*getBlockTexture)(Block *b, Direction dir, int x, int y, int z, u8 meta);
	void (*registerIcons)(Block *b);
} BlockVtable;

struct Block {
	BlockVtable *vptr;			 // virtual table for inheriting block classes.
	char name[64];				 // for texture path
	BlockId id;					 // block id, matches enum index.
	u8 renderType;				 // id for type of block/mesh
	float hardness;				 // how many hits to break block
	float resistance;			 // explosion resistance
	u8 lightness;				 // lightness, 0 to 15
	Box bounds;					 // bounds
	MaterialType material;		 // Material Enum
	u16 icon;					 // index of texture icon
	bool useNeighborBrightness;	 // set by code(see Blocks_Init)
};

extern Texture_Map gTexMapBlock;

Block *Block_InitWithBounds(const char *name, BlockId id, float resistance, float hardness, MaterialType material, Box bounds);
Block *Block_Init(const char *name, BlockId id, float resistance, float hardness, MaterialType material);

void Block_SetResistance(Block *block, float v);
void Block_SetHardness(Block *block, float v);
void Block_SetBounds(Block *block, float3 from, float3 to);
void Block_SetLightness(Block *block, u8 v);

bool Block_ShouldSideBeRendered(Block *block, int x, int y, int z, Direction dir);
const Material *Block_GetMaterial(Block *block);

static inline bool Block_RenderAsNormalBlock(Block *block) {
	return block->vptr->renderAsNormalBlock();
}
static inline bool Block_IsOpaqueCube(Block *block) {
	return block->vptr->isOpaqueCube();
}
static inline u8 Block_GetRenderType(Block *block) {
	return block->vptr->getRenderType();
}
#define colR(c) ((c >> 16) & 0xff)
#define colG(c) (((c) >> 8) & 0xff)
#define colB(c) ((c)&0xff)
static inline void Block_GetBlockColor(Block *b, Direction dir, int x, int y, int z, u8 meta, u8 out[]) {
	u32 color = COLORS[b->vptr->getBlockColor(b, dir, x, y, z, meta)];

	u8 brightness = 6;
	if (b->id == BLOCK_GRASS && dir == Direction_Top)
		brightness = 15;
	brightness *= 17;

	out[0] = (colR(color) * brightness + brightness) / 255;
	out[1] = (colG(color) * brightness + brightness) / 255;
	out[2] = (colB(color) * brightness + brightness) / 255;
}
static inline void Block_GetBlockTexture(Block *b, Direction dir, int x, int y, int z, u8 meta, s16 out_uv[]) {
	const Icon icon = gTexMapBlock.icons[b->vptr->getBlockTexture(b, dir, x, y, z, meta)];

	out_uv[0] = icon.u;
	out_uv[1] = icon.v;
}
static inline u16 Block_GetIcon(Block *b, Direction dir, u8 meta) {
	return b->vptr->getBlockTexture(b, dir, 0, 0, 0, meta);
}
static inline float Block_GetHardness(Block *block) {
	return block->hardness;
}
static inline float Block_GetResistance(Block *block) {
	return block->resistance;
}