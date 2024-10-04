#pragma once

#include "client/Crash.h"
#include "client/renderer/texture/TextureMap.h"
#include "core/Direction.h"
#include "util/math/VecMath.h"
#include "world/level/material/Material.h"
#include "world/phys/Collision.h"

#include "Blocks.h"

#define COLOR_WHITE 16777215

typedef struct Block Block;

typedef struct {
	u8 (*getRenderType)();
	u32 (*getBlockColor)(Block *b, Direction dir, int x, int y, int z, u8 meta);
	u32 (*getItemColor)(Direction dir, u8 meta);
} BlockVtable;

struct Block {
	BlockVtable *vptr;		// virtual table for inheriting block classes.
	const char *name;		// for texture path
	BlockId id;				// block id, matches enum index.
	u8 renderType;			// id for type of block/mesh
	float hardness;			// how many hits to break block
	float resistance;		// explosion resistance
	u8 lightness;			// lightness, 0 to 15
	Box bounds;				// bounds
	MaterialType material;	// Material Enum
	MapColor mapColor;
	u16 icon;					 // index of texture icon
	bool useNeighborBrightness;	 // set by code(see Blocks_Init)
	bool opaque;				 // if yes, dont allow light to pass thru
	bool solidBlock;
	bool hasRandomVariants;
};

extern Texture_Map gTexMapBlock;

Block *Block_InitWithBounds(const char *name, BlockId id, float resistance, float hardness, MaterialType material, MapColor mapColor,
							Box bounds);
Block *Block_Init(const char *name, BlockId id, float resistance, float hardness, MaterialType material, MapColor mapColor);

void Block_SetResistance(Block *block, float v);
void Block_SetHardness(Block *block, float v);
void Block_SetBounds(Block *block, float3 from, float3 to);
void Block_SetLightness(Block *block, u8 v);
void Block_SetNotOpaque(Block *b);
void Block_SetNotSolidBlock(Block *b);

void Block_GetBlockColor(Block *b, Direction dir, int x, int y, int z, u8 meta, u8 out[]);
void Block_GetItemColor(Block *b, Direction dir, u8 meta, u8 out[]);

static inline u8 Block_GetRenderType(Block *block) {
	return block->vptr->getRenderType();
}
static inline const Material *Block_GetMaterial(Block *block) {
	return &MATERIALS[block->material];
}