#pragma once

#include "BlockElementFace.h"

#include "util/math/VecMath.h"

typedef struct {
	int3 from, to;
	BlockElementFace faces[6];
	bool shade;

} BlockElement;

typedef struct mpack_node_t mpack_node_t;

BlockElement BlockElement_Deserialize(mpack_node_t element);