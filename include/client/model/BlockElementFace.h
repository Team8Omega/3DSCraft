#pragma once

#include "BlockFaceUV.h"
#include "core/Direction.h"

typedef struct {
	Direction cullDir;
	s8 tintIndex;
	const char* texture;
	BlockFaceUV uv;
} BlockElementFace;

BlockElementFace BlockElementFace_Deserialize(mpack_node_t face);