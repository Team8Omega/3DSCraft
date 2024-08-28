#pragma once

#include "client/model/VertexFmt.h"

typedef struct {
	WorldVertex** faces;
	size_t numFaces;
} BakedModel;