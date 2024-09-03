#pragma once

#include "core/VertexFmt.h"
#include "resources/model/BakedModel.h"

void ModelManager_Init();

BakedModel* ModelManager_GetModel(const char* name);