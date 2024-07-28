#pragma once

#include "client/model/Cube.h"

typedef struct {
	C3D_Mtx rootMatrix;

	u8 cubeNum;
	Cube** cubes;

	C3D_Tex* texture;
} CubeModel;

typedef struct {
	u8 cubeNum;
	CubeModelDef* modeldef;

	C3D_Tex* texture;
} CubeModelRaw;

CubeModel* CubeModel_Init(CubeModelRaw* model);

#define CUBE_MAX_NUM 16	 // adjust as needed.

static inline CubeModel* createModel(CubeModelDef* modeldef, C3D_Tex* texture) {
	u8 numModels = 0;
	for (u8 i = 0; i < CUBE_MAX_NUM; ++i) {
		if (modeldef->cubes[i].to[0] != 0)
			numModels++;
	}

	CubeModelRaw preModel = { .cubeNum = numModels, .modeldef = &*modeldef, .texture = texture };

	return CubeModel_Init(&preModel);
}
static inline void CubeModel_Clean(CubeModelRaw* model) {
	if (model == NULL)
		return;

	linearFree(model);
}
static inline void CubeModel_SetTexture(CubeModel* m, C3D_Tex* tex) {
	m->texture = tex;
}

void CubeModel_Draw(CubeModel* model, int shaderUniform, C3D_Mtx* matrix);

void CubeModel_Deinit(CubeModel* model);

void CubeModel_Reset(CubeModel* model);

void CubeModel_SetPos(CubeModel* model, float3 pos);

void CubeModel_SetRotY(CubeModel* model, float rotY);
void CubeModel_SetRot(CubeModel* model, float3 rot);
