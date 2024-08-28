#include "resources/model/ModelBakery.h"

#include <3ds.h>
#include <unistd.h>
#include <vec/vec.h>

#include "util/Paths.h"
#include "util/SerialUtils.h"
#include "util/StringUtils.h"

static vec_t(BlockModel) sUnbakedBlocks;

void ModelBakery_Init() {
	vec_init(&sUnbakedBlocks);
}

static BlockModel loadModel(const char* name) {
	const char* path = String_ParsePackName(PACK_VANILLA, PATH_PACK_MODELS, String_AddSuffix(name, ".mp"));

	BlockModel model = {};
	if (access(path, F_OK) == -1) {
		Crash("File not found\nModelfile missing for %s\n", name);
		return model;
	}

	mpack_tree_t levelTree = serial_get_start(path);
	model				   = BlockModel_Deserialize(serial_get_root(&levelTree), name);

	return model;
}

BlockModel* ModelBakery_GetModel(const char* name) {
	u32 hashName = String_Hash(name);
	if (sUnbakedBlocks.length > 0) {
		int index;
		BlockModel* model;
		vec_foreach_ptr(&sUnbakedBlocks, model, index) {
			if (model->hash == hashName)
				return &sUnbakedBlocks.data[index];
		}
	}

	BlockModel model = loadModel(name);

	BlockModel* parent = NULL;
	if (model.parentName[0] != '\0') {
		parent = ModelBakery_GetModel(model.parentName);
	}

	BlockModel newModel;
	if (parent) {
		newModel = *(parent);
		strcpy(newModel.name, model.name);
		strcpy(newModel.parentName, model.parentName);
		newModel.guiLight = model.guiLight != GUILIGHT_NONE ? model.guiLight : parent->guiLight;
		newModel.hasAmbientOcclusion =
			model.hasAmbientOcclusion != AMBIENTOCC_NONE ? model.hasAmbientOcclusion : parent->hasAmbientOcclusion;
		newModel.hash = hashName;
		if (model.elementNum > 0) {
			newModel.elementNum = model.elementNum;
			newModel.elements	= model.elements;
		} else {
			newModel.elementNum = parent->elementNum;
			newModel.elements	= model.elements;
		}
	} else {
		newModel = model;
	}

	vec_push(&sUnbakedBlocks, newModel);

	return &sUnbakedBlocks.data[sUnbakedBlocks.length - 1];
}