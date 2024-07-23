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
	CubeRaw** cubes;

	C3D_Tex* texture;
} CubeModelRaw;

CubeModel* CubeModel_Init(CubeModelRaw* model);

static inline CubeModel* createModel(CubeRaw* models, u8 numModels, C3D_Tex* texture) {
	CubeRaw** pointers = (CubeRaw**)malloc(sizeof(CubeRaw*) * numModels);
	if (!pointers)
		return NULL;

	for (u8 i = 0; i < numModels; ++i) {
		pointers[i] = &models[i];
	}

	CubeModelRaw preModel = { .cubeNum = numModels, .cubes = pointers, .texture = texture };

	return CubeModel_Init(&preModel);
}
static inline void CubeModel_Clean(CubeModelRaw* model) {
	if (model == NULL)
		return;

	for (u8 i = 0; i < model->cubeNum; ++i)
		Cube_Clean(model->cubes[i]);

	linearFree(model->cubes);
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
