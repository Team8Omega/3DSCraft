#include "client/renderer/block/model/BlockStates.h"

#include <3ds.h>
#include <util/SerialUtils.h>

#include "resources/model/ModelManager.h"
#include "util/random/WeightedRandom.h"

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
			mpack_node_t varNode	= serial_get_arrayNodeAt(stateNode, 0);
			state.variants[i].index = i;
			if (serial_get_node(varNode, "weight").data->type == mpack_type_float)
				weights[i] = serial_get(varNode, float, "weight", 1.f);
			else
				weights[i] = (float)serial_get(varNode, uint, "weight", 1);
		}
		state.weightedRandom = WeightedRandom_Init(size, weights);
	} else {
		state.variants[0].index = 0;
		state.weightedRandom	= NULL;
	}
	return state;
}

void BlockStates_Decompile() {
}