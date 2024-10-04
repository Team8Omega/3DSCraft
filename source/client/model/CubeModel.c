#include "client/model/CubeModel.h"

#include "client/Camera.h"
#include "client/Crash.h"
#include "client/renderer/texture/TextureMap.h"

CubeModel* CubeModel_Init(CubeModelRaw* in) {
	if (!in) {
		Crash(0, "Passed raw Model is NULL!");
		return NULL;
	}

	CubeModel* model = (CubeModel*)malloc(sizeof(CubeModel));

	model->cubeNum = in->cubeNum;

	model->cubes = (Cube**)malloc(model->cubeNum * sizeof(Cube*));

	for (u8 i = 0; i < model->cubeNum; ++i) {
		if (!in->modeldef->cubes[i].size[0]) {
			Crash(0, "Cube %d for Model is NULL!\n Total: %d\n in->modeldef->cubes: %08x", i, model->cubeNum, in->modeldef->cubes);
		}
		model->cubes[i] = Cube_Init(&in->modeldef->cubes[i], in->modeldef->dimensions[0], in->modeldef->dimensions[1]);
	}

	CubeModel_Clean(in);

	return model;
}

void CubeModel_Deinit(CubeModel* model) {
	if (model == NULL)
		return;

	for (u8 i = 0; i < model->cubeNum; ++i) {
		Cube_Deinit(model->cubes[i]);
	}

	linearFree(model->cubes);
	linearFree(model);
}

void CubeModel_Draw(CubeModel* model, C3D_Mtx* movement) {
	if (!model)
		Crash(0, "MODEL == NULL");

#define modelScale 0.05f
	Mtx_Scale(movement, modelScale, modelScale, modelScale);

	C3D_CullFace(GPU_CULL_FRONT_CCW);

	for (u8 i = 0; i < model->cubeNum; ++i) {
		Cube* cube = model->cubes[i];
		Cube_Draw(cube, movement);
	}

	C3D_CullFace(GPU_CULL_BACK_CCW);
}