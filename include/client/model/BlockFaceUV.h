#pragma once

#include <mpack/mpack.h>

typedef struct {
	u16 uvs[4];
	s16 rotation;
} BlockFaceUV;

BlockFaceUV BlockFaceUV_Deserialize(mpack_node_t face);