#pragma once

#include "client/renderer/block/model/BlockModel.h"
#include "resources/model/BakedModel.h"

void ModelBakery_Init();

BakedModel* ModelBakery_GetModel(const char* name);