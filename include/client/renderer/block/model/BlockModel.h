#pragma once

#include "BlockElement.h"

typedef struct mpack_node_t mpack_node_t;

enum {
	GUILIGHT_NONE,
	GUILIGHT_FRONT,
	GUILIGHT_SIDE
};

typedef u8 GuiLight;

typedef struct {
	u32 hash;
	const char* name;
	const char* parentName;
	BlockElement* elements;
	u8 elementNum;
	GuiLight guiLight;
	bool hasAmbientOcclusion;
} BlockModel;

BlockModel BlockModel_Deserialize(mpack_node_t root, const char* name);

int BlockModel_GetDependencies(char** out_ids[]);