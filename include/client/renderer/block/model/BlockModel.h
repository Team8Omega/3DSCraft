#pragma once

#include "BlockElement.h"

typedef struct mpack_node_t mpack_node_t;

enum {
	GUILIGHT_NONE,
	GUILIGHT_FRONT,
	GUILIGHT_SIDE
};
enum {
	AMBIENTOCC_NONE,
	AMBIENTOCC_TRUE,
	AMBIENTOCC_FALSE
};

typedef u8 GuiLight;
typedef u8 AmbientOcc;

typedef struct {
	u32 hash;
	char name[64];
	char parentName[64];
	BlockElement* elements;
	u8 elementNum;
	GuiLight guiLight;
	AmbientOcc hasAmbientOcclusion;
} BlockModel;

BlockModel BlockModel_Deserialize(mpack_node_t root, const char* name);

int BlockModel_GetDependencies(char** out_ids[]);