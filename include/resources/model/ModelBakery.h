#pragma once

#include "client/Crash.h"

#include "client/renderer/block/model/BlockModel.h"
#include "resources/model/BakedModel.h"

void ModelBakery_Init();
void ModelBakery_Deinit();

BakedModel* ModelBakery_GetModel(const char* name);

static inline void namePrefixMC(char* name) {
	if (!name)
		Crash(0, "Recieved NULL Name!");

	if (strncmp(name, "minecraft:", 10) == 0 && strlen(name) >= 10) {
		memmove(name, name + 10, strlen(name) - 10 + 1);
	}
}