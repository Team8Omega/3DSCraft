#pragma once

#include "client/model/Cube.h"

typedef struct {
	u8 cubeNum;
	Cube** cubes;
} CubeModel;

typedef struct {
	u8 cubeNum;
	CubeModelDef* modeldef;
} CubeModelRaw;

CubeModel* CubeModel_Init(CubeModelRaw* model);

#define CUBE_MAX_NUM 16	 // adjust as needed.

static inline CubeModel* createModel(CubeModelDef* modeldef) {
	u8 numModels = 0;
	for (u8 i = 0; i < CUBE_MAX_NUM; ++i) {
		if (modeldef->cubes[i].size[0] != 0)
			numModels++;
	}

	CubeModelRaw preModel = { .cubeNum = numModels, .modeldef = &*modeldef };

	return CubeModel_Init(&preModel);
}
static inline void CubeModel_Clean(CubeModelRaw* model) {
	if (model == NULL)
		return;

	linearFree(model->modeldef->cubes);

	linearFree(model);
}

void CubeModel_Draw(CubeModel* model, C3D_Mtx* movement);

void CubeModel_Deinit(CubeModel* model);