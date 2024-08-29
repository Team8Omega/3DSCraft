#include "resources/model/ModelManager.h"

#include <vec/vec.h>

#include "resources/model/ModelBakery.h"

static vec_t(BakedModel) sBuffer_BakedModels;

void ModelManager_Init() {
	ModelBakery_Init();
	vec_init(&sBuffer_BakedModels);
}

BakedModel* ModelManager_GetModel(const char* name) {
	return ModelBakery_GetModel(name);
}