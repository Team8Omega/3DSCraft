#include "resources/model/ModelBakery.h"

#include <3ds.h>
#include <unistd.h>
#include <vec/vec.h>

#include "util/Paths.h"
#include "util/SerialUtils.h"
#include "util/StringUtils.h"

static vec_t(BlockModel) sUnbakedBlocks;

static BlockModel loadModel(const char* name) {
	const char* path = String_ParsePackName(PACK_VANILLA, PATH_PACK_MODELS, String_AddSuffix(name, ".mp"));

	BlockModel model = {};
	if (access(path, F_OK) == -1) {
		Crash("File not found: %s\n at loadModel()", path);
		return model;
	}

	mpack_tree_t levelTree = serial_get_start(path);
	model				   = BlockModel_Deserialize(serial_get_root(&levelTree), name);

	return model;
}

BlockModel ModelBakery_GetModel(const char* name) {
	u32 hashName = String_Hash(name);
	if (sUnbakedBlocks.length > 0) {
		int index;
		BlockModel* model;
		vec_foreach_ptr(&sUnbakedBlocks, model, index) {
			if (model->hash == hashName)
				return sUnbakedBlocks.data[index];
		}
	}

	u32 startTime	 = svcGetSystemTick();
	BlockModel model = loadModel(name);
	u32 endTime		 = svcGetSystemTick();

	float elapsedMs = (float)(endTime - startTime) / (float)CPU_TICKS_PER_MSEC;

	Crash("Loaded Model! Took %.3f ms.\n\nName: %s,\nParentName: %s,\nElementNum: %lu", elapsedMs, model.name, model.parentName,
		  model.elementNum);

	vec_push(&sUnbakedBlocks, model);

	return model;
}