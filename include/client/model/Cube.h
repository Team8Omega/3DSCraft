#pragma once

#include <citro3d.h>
#include <stdlib.h>

#include "client/model/VertexFmt.h"
#include "util/math/VecMath.h"

#define CUBE_VERTICE_NUM 6 * 6

typedef struct {
	float from[3], to[3];
	s16 dimensions[2];
	s16 faceUV[6][4];  // pixel based coordinates
	float position[3], rotation[3];
} CubeRaw;

typedef struct {
	u16 vboIdx;
	C3D_Mtx localMatrix;
	C3D_Mtx initialMatrix;
	WorldVertex* vertices;
} Cube;

static inline void Cube_Clean(CubeRaw* model) {
	if (model == NULL)
		return;

	linearFree(model);
}

Cube* Cube_Init(CubeRaw* in);

void Cube_Draw(Cube* cube, int shaderUniform, C3D_Mtx* matrix);

void Cube_Deinit(Cube* cube);

void Cube_SetPos(Cube* cube, float3 pos);

void Cube_SetRot(Cube* cube, float3 rot);

void Cube_Reset(Cube* cube);

void Cube_ResetToInit(Cube* cube);

void Cube_InitVBOs();

void Cube_DeinitVBOs();