#include "client/model/CubeModel.h"

#include "client/Camera.h"
#include "client/Crash.h"
#include "client/renderer/texture/TextureMap.h"

CubeModel* CubeModel_Init(CubeModelRaw* in) {
	if (!in) {
		Crash("Passed raw Model is NULL!");
		return NULL;
	} else if (!in->modeldef->cubes[0].from[0]) {
		Crash("Passed cubes for raw Model is NULL");
		return NULL;
	}

	CubeModel* model = linearAlloc(sizeof(CubeModel));
	if (!model) {
		Crash("Could not allocate memory for model");
		return NULL;
	}
	model->texture = in->texture;

	model->cubeNum = in->cubeNum;

	model->cubes = linearAlloc(model->cubeNum * sizeof(Cube*));
	if (!model->cubes) {
		linearFree(model);
		Crash("Could not allocate memory for model cubes\n Amount: %d", model->cubeNum);
		return NULL;
	}

	for (u8 i = 0; i < model->cubeNum; ++i) {
		if (!in->modeldef->cubes[i].to[0]) {
			Crash("Cube %d for Model is NULL!\n Total: %d\n in->modeldef->cubes: %08x", i, model->cubeNum, in->modeldef->cubes);
		}
		model->cubes[i] = Cube_Init(&in->modeldef->cubes[i], in->modeldef->dimensions[0], in->modeldef->dimensions[1]);
	}

	CubeModel_Clean(in);

	if (!model) {
		Crash("Model generation returns NULL.");
		return NULL;
	}

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

void CubeModel_Draw(CubeModel* model, int projectionUniform, C3D_Mtx* projection) {
	C3D_Mtx matrix;
	Mtx_Identity(&matrix);
	Mtx_Multiply(&matrix, projection, &model->rootMatrix);
#define modelScale 0.05f
	Mtx_Scale(&matrix, modelScale, modelScale, modelScale);

	if (model->texture == NULL) {
		Crash("CubeRaw TEXTURE IS NULL");  // todo: broken texture handling
		return;
	}

	C3D_CullFace(GPU_CULL_NONE);
	C3D_AlphaTest(true, GPU_GREATER, 0);
	C3D_TexBind(0, model->texture);

	for (u8 i = 0; i < model->cubeNum; ++i) {
		Cube* cube = model->cubes[i];
		Cube_Draw(cube, projectionUniform, &matrix);
	}

	C3D_AlphaTest(false, GPU_GREATER, 0);
	C3D_CullFace(GPU_CULL_BACK_CCW);
}

void CubeModel_Reset(CubeModel* m) {
	Mtx_Identity(&m->rootMatrix);
}

void CubeModel_SetPos(CubeModel* m, float3 pos) {
	Mtx_Translate(&m->rootMatrix, pos.x, pos.y, pos.z, true);
}

void CubeModel_SetRot(CubeModel* m, float3 rot) {
	Mtx_RotateX(&m->rootMatrix, rot.x, true);
	Mtx_RotateX(&m->rootMatrix, rot.y, true);
	Mtx_RotateX(&m->rootMatrix, rot.z, true);
}
void CubeModel_SetRotY(CubeModel* m, float rotY) {
	Mtx_RotateY(&m->rootMatrix, rotY, true);
}