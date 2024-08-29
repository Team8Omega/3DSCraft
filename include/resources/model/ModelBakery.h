#pragma once

#include "client/renderer/block/model/BlockModel.h"
#include "resources/model/BakedModel.h"

void ModelBakery_Init();
void ModelBakery_Deinit();

BakedModel* ModelBakery_GetModel(const char* name);