#include "world/level/block/states/BlockStates.h"

#include <3ds.h>
#include <unistd.h>

#include "resources/model/ModelBakery.h"
#include "resources/model/ModelManager.h"
#include "util/Paths.h"
#include "util/SerialUtils.h"
#include "util/StringUtils.h"
#include "util/random/WeightedRandom.h"
#include "world/level/block/Block.h"

BlockState BLOCKSTATES[BLOCK_COUNT];

static BlockState getState(mpack_node_t stateNode) {
	BlockState state = {};

	size_t size	 = 1;
	bool isArray = stateNode.data->type == mpack_type_array;
	if (isArray) {
		size = serial_get_arrayLength(stateNode);
	}
	state.variants	  = linearAlloc(sizeof(BlockStateVariant) * size);
	state.numVariants = size;
	if (isArray) {
		float weights[size];
		for (size_t i = 0; i < size; ++i) {
			mpack_node_t varNode = serial_get_arrayNodeAt(stateNode, 0);

			char name[64];
			serial_get_cstr(varNode, "model", name, 64);

			if (strncmp(name, "minecraft:", 10) == 0) {
				memmove(name, name + 10, strlen(name) - 10 + 1);
			}

			state.variants[i].model = ModelManager_GetModel(name);
			state.variants[i].index = i;
			if (serial_get_node(varNode, "weight").data->type == mpack_type_float)
				weights[i] = serial_get(varNode, float, "weight", 1.f);
			else
				weights[i] = (float)serial_get(varNode, uint, "weight", 1);
		}
		state.weightedRandom = WeightedRandom_Init(size, weights);
	} else {
		char name[64];
		serial_get_cstr(stateNode, "model", name, 64);

		if (strncmp(name, "minecraft:", 10) == 0) {
			memmove(name, name + 10, strlen(name) - 10 + 1);
		}

		state.variants[0].model = ModelManager_GetModel(name);
		state.variants[0].index = 0;
		state.weightedRandom	= NULL;
	}
	return state;
}

void BlockStates_Decompile() {
	u32 startTime = svcGetSystemTick();

	for (size_t i = 0; i < BLOCK_COUNT; ++i) {
		Block* block = BLOCKS[i];

		char* name = String_ParsePackName(PACK_VANILLA, PATH_PACK_BLOCK, block->name);
		name	   = realloc(name, strlen(name) + 4);
		strcat(name, ".mp");

		if (access(name, F_OK)) {
			Crash("BlockState file not found\nCould not open file for BlockState\nName:%s\nPath:%s", block->name, name);
			return;
		}

		mpack_tree_t tree = serial_get_start(name);
		mpack_node_t root = serial_get_root(&tree);

		if (serial_has(root, "multipart"))
			Crash("BlockState MultiPart unsupported\nNot added yet, please contact developer!\n\nAt %s", name);
		else if (serial_has(root, "variants")) {
			mpack_node_t data = serial_get_node(root, "variants");
			size_t size		  = serial_get_mapLength(data);
			for (size_t i = 0; i < size; ++i) {
				char keyname[64];
				serial_get_keyname(data, i, keyname, 64);
				mpack_node_t variant = serial_get_node(data, keyname);
				serial_get_error(variant, "Pre-BlockState");
				BLOCKSTATES[i] = getState(variant);
			}
		} else {
			char str[64];
			serial_get_keyname(root, 0, str, 64);
			Crash("BlockState Unknown type!\nType could not be detected or was not implemented.\nName: %s\nAt: %s\nBlockName: %s", str,
				  name, block->name);
			return;
		}
		serial_get_error(root, "Post-BlockState");
	}
	u32 endTime		= svcGetSystemTick();
	float elapsedMs = ((float)(endTime / (float)CPU_TICKS_PER_MSEC) - (float)(startTime / (float)CPU_TICKS_PER_MSEC));

	ModelBakery_Deinit();

	Crash("BlockStates loaded! took %.0f ms.", elapsedMs);
}