#pragma once

#include "client/renderer/texture/TextureMap.h"
#include "core/Direction.h"
#include "util/math/VecMath.h"
#include "world/level/block/material/Material.h"
#include "world/phys/Collision.h"

typedef struct Block Block;

typedef struct {
	bool (*renderAsNormalBlock)();
	bool (*isOpaqueCube)();
	u8 (*getRenderType)();
	void (*textureLoad)(Block *b);
	int (*getBlockColor)(Direction dir, u8 meta);
	Texture_MapIcon (*getBlockTexture)(Block *b, Direction dir, u8 meta);
} BlockVtable;

struct Block {
	BlockVtable *vptr;		   // virtual table for inheriting block classes.
	char textureName[64];	   // for texture path
	char unlocalizedName[64];  // for localized string
	u16 id;					   // block id, matches enum index.
	float hardness;			   // how many hits to break block
	float resistance;		   // explosion resistance
	Box bounds;				   // bounds
	MaterialType material;	   // Material Enum
	u16 icon;				   // index of texture icon
};

extern Texture_Map gTexMapBlock;

Block *Block_Init(const char *unlocalizedName, const char *textureName, u16 id, float resistance, float hardness, MaterialType material,
				  Box bounds);

void Block_SetResistance(Block *block, float v);
void Block_SetHardness(Block *block, float v);
void Block_SetBounds(Block *block, float3 from, float3 to);

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
static inline void Block_GetBlockColor(Block *b, Direction dir, u8 meta, u8 out_rgb[]) {
	const int color = b->vptr->getBlockColor(dir, meta);

	out_rgb[0] = colR(color);
	out_rgb[1] = colG(color);
	out_rgb[2] = colB(color);
}
static inline void Block_GetBlockTexture(Block *b, Direction dir, u8 meta, s16 out_uv[]) {
	const Texture_MapIcon icon = b->vptr->getBlockTexture(b, dir, meta);

	out_uv[0] = icon.u;
	out_uv[1] = icon.v;
}
static inline float Block_GetHardness(Block *block) {
	return block->hardness;
}
static inline float Block_GetResistance(Block *block) {
	return block->resistance;
}