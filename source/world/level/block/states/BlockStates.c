#include "world/level/block/states/BlockStates.h"

#include <3ds.h>
#include <malloc.h>
#include <unistd.h>

#include "client/gui/DebugUI.h"
#include "resources/model/ModelBakery.h"
#include "resources/model/ModelManager.h"
#include "util/Paths.h"
#include "util/SerialUtils.h"
#include "util/StringUtils.h"
#include "util/random/WeightedRandom.h"
#include "world/level/block/Block.h"

BlockStateHolder BLOCKSTATES[BLOCK_COUNT];
extern u32 __ctru_linear_heap_size;

static BlockState getState(mpack_node_t stateNode) {
	BlockState state = {};

	size_t size	 = 1;
	bool isArray = stateNode.data->type == mpack_type_array;
	if (isArray) {
		size = serial_get_arrayLength(stateNode);
	}
	state.variants	 = linearAlloc(sizeof(BlockStateVariant) * size);
	state.variantNum = size;
	if (isArray) {
		float weights[size];
		for (size_t i = 0; i < size; ++i) {
			mpack_node_t varNode = serial_get_arrayNodeAt(stateNode, 0);

			char name[64];
			serial_get_cstr(varNode, "model", name, 64);

			namePrefixMC(name);

			serial_get_error(varNode, "Pre-BlockState_ModelLoading", true);
			state.variants[i].model = ModelManager_GetModel(name);
			serial_get_error(varNode, "Post-BlockState_ModelLoading", true);

			size_t vNum = state.variants[i].model->numVertex;
			if (vNum > state.vertexNum)
				state.vertexNum = vNum;

			state.variants[i].index = i;

			if (serial_get_node(varNode, "weight").data->type == mpack_type_float)
				weights[i] = serial_get(varNode, float, "weight", 1.f);
			else
				weights[i] = (float)serial_get(varNode, uint, "weight", 1);
		}
		state.random = WeightedRandom_Init(size, weights);
	} else {
		char name[64];
		serial_get_cstr(stateNode, "model", name, 64);

		namePrefixMC(name);

		state.variants[0].model = ModelManager_GetModel(name);
		state.variants[0].index = 0;
		state.vertexNum			= state.variants[0].model->numVertex;

		state.random = NULL;
	}
	return state;
}

void BlockStates_Decompile() {
	u32 startTime	 = svcGetSystemTick();
	u32 startSpace	 = linearSpaceFree();
	size_t startHeap = mallinfo().uordblks;

	for (size_t i = 0; i < BLOCK_COUNT; ++i) {
		Block* block = BLOCKS[i];

		char* name = String_ParsePackName(PACK_VANILLA, PATH_PACK_BLOCK, block->name);
		name	   = realloc(name, strlen(name) + 4);
		strcat(name, ".mp");

		if (access(name, F_OK)) {
			Crash(0, "BlockState file not found\nCould not open file for BlockState\nName:%s\nPath:%s", block->name, name);
			return;
		}

		mpack_tree_t tree = serial_get_start(name);
		mpack_node_t root = serial_get_root(&tree);

		u32 typehash = 0;
		{
			char keyname[32];
			serial_get_keyname(root, 0, keyname, 32);
			typehash = String_Hash(keyname);
		}
		switch (typehash) {
			case (u32)249898626611028071:  // multipart
				Crash(0, "BlockState MultiPart unsupported\nNot added yet, please contact developer!\n\nAt %s", name);
				break;

			case (u32)7573043612985229:	 // variants
				mpack_node_t data = serial_get_node(root, "variants");
				size_t size		  = serial_get_mapLength(data);

				BlockStateHolder holder = { .stateNum = size };

				holder.states = malloc(sizeof(BlockState) * size);

				for (size_t j = 0; j < size; ++j) {
					char keyname[64];
					serial_get_keyname(data, j, keyname, 64);

					mpack_node_t variant = serial_get_node(data, keyname);
					serial_get_error(variant, "Pre-BlockState", true);

					holder.states[j] = getState(variant);

					serial_get_error(variant, "Mid-BlockState", true);

					if (holder.states[j].random)
						BLOCKS[i]->hasRandomVariants = true;
				}
				BLOCKSTATES[i] = holder;
				break;

			default:
				char str[64];
				serial_get_keyname(root, 0, str, 64);
				Crash(0, "BlockState Unknown type!\nType could not be detected or was not implemented.\nName: %s\nAt: %s\nBlockName: %s",
					  str, name, block->name);
				return;
		}
		serial_get_error(root, "Post-BlockState", true);
	}
	u32 endTime		= svcGetSystemTick();
	float elapsedMs = ((float)(endTime / (float)CPU_TICKS_PER_MSEC) - (float)(startTime / (float)CPU_TICKS_PER_MSEC));

	ModelBakery_Deinit();

	u32 endSpace  = linearSpaceFree();
	u32 usedSpace = startSpace - endSpace;

	size_t endHeap	= mallinfo().uordblks;
	size_t usedHeap = endHeap - startHeap;

	DebugUI_Log("BlockStates loaded! took %.0f ms - Used now: Lin: %ukB, Heap: %zukB, All: %ukB", CRASH_ALLOC, elapsedMs, usedSpace >> 10,
				usedHeap >> 10, (usedSpace + usedHeap) >> 10);
}

BlockStateVariant* BlockState_Get(BlockId blockId, u8 index) {
	BlockState* statePtr = &BLOCKSTATES[blockId].states[index];

	u32 varIdx = WeightedRandom_GetRandom(statePtr->random);

	return &statePtr->variants[varIdx];
}
