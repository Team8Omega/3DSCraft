#pragma once

#include <citro3d.h>
#include <stdlib.h>

#include "client/model/CubeRaw.h"
#include "client/model/VertexFmt.h"
#include "util/math/VecMath.h"

#define CUBE_VERTICE_NUM 6 * 6

typedef struct {
	u16 vboIdx;
	C3D_Mtx localMatrix;
	float3 rotationPoint;
	WorldVertex* vertices;
} Cube;

Cube* Cube_Init(const CubeRaw* in, s16 texwidth, s16 texheight);

void Cube_Draw(Cube* cube, int shaderUniform, C3D_Mtx* matrix);

void Cube_Deinit(Cube* cube);

void Cube_SetPos(Cube* cube, float3 pos);

void Cube_SetRot(Cube* cube, float3 rot);

void Cube_SetRotPoint(Cube* cube, float3 pos);

void Cube_Reset(Cube* cube);

void Cube_InitVBOs();

void Cube_DeinitVBOs();