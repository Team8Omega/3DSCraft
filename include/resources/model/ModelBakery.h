#pragma once

#include "client/renderer/block/model/BlockModel.h"

void ModelBakery_Init();

BlockModel* ModelBakery_GetModel(const char* name);