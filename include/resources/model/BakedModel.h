#pragma once

#include "client/model/VertexFmt.h"

typedef struct {
	WorldVertex** vertex;
	size_t numFaces;
} BakedModel;