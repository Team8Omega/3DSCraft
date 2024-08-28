#pragma once

#include "BlockFaceUV.h"
#include "core/Direction.h"

typedef struct {
	char texture[32];
	BlockFaceUV uv;
	s8 tintIndex;
	bool exists;
} BlockElementFace;

BlockElementFace BlockElementFace_Deserialize(mpack_node_t face);