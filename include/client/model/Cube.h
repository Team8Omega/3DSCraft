#pragma once

#include <citro3d.h>
#include <stdlib.h>

#include "client/model/CubeRaw.h"
#include "client/model/VertexFmt.h"
#include "util/math/VecMath.h"

#define CUBE_VERTICE_NUM 6 * 6

typedef struct {
	u16 vboIdx;
	float3 rotationPoint;
	WorldVertex* vertices;
} Cube;

Cube* Cube_Init(const CubeRaw* in, s16 texwidth, s16 texheight);

void Cube_Draw(Cube* cube, C3D_Mtx* matrix);

void Cube_Deinit(Cube* cube);

void Cube_InitVBOs();  // needs to be called AFTER all cubes have been built.

void Cube_DeinitVBOs();