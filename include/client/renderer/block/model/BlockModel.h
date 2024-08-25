#pragma once

#include "BlockElement.h"

typedef enum {
	GUILIGHT_FRONT,
	GUILIGHT_SIDE,
	GUILIGHT_COUNT
} GuiLight;

typedef struct mpack_node_t mpack_node_t;

typedef struct {
	const char* name;
	const char* parentName;
	bool hasAmbientOcclusion;
	GuiLight guiLight;
	u8 elementNum;
	BlockElement* elements;
} BlockModel;

BlockModel BlockModel_Deserialize(mpack_node_t root);