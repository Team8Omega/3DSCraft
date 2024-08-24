#include "client/renderer/block/model/BlockElement.h"

#include <mpack/mpack.h>

#include "client/renderer/block/model/BlockElementFace.h"
#include "core/Direction.h"
#include "util/SerialUtils.h"
#include "util/math/VecMath.h"

#define MIN_EXTENT -16.f
#define MAX_EXTENT 32.f

static BlockElementFace sBuffer_Faces[6];

static int3 parseVector3(mpack_node_t e, const char* keyName) {
	mpack_node_t array = serial_get_node(e, keyName);
	if (serial_get_arrayLength(array) != 3)
		Crash("Expected 3 \'%s\' values, found %zu\n\'%s\' is of type %s", keyName, serial_get_arrayLength(array), keyName,
			  mpack_type_to_string(array.data->type));

	int3 val;
	for (u8 i = 0; i < 3; ++i) {
		val.v[i] = serial_get_at(array, int, i);
	}
	return val;
}

static int3 parseToPos(mpack_node_t e) {
	int3 vec = parseVector3(e, "to");
	if (vec.x < MIN_EXTENT || vec.y < MIN_EXTENT || vec.z < MIN_EXTENT || vec.x >= MAX_EXTENT || vec.y >= MAX_EXTENT || vec.z >= MAX_EXTENT)
		Crash("\'to\' specifier exceeds the allowed boundaries: [%f,%f,%f]", vec.x, vec.y, vec.z);

	return vec;
}
static int3 parseFromPos(mpack_node_t e) {
	int3 vec = parseVector3(e, "from");
	if (vec.x < MIN_EXTENT || vec.y < MIN_EXTENT || vec.z < MIN_EXTENT || vec.x >= MAX_EXTENT || vec.y >= MAX_EXTENT || vec.z >= MAX_EXTENT)
		Crash("\'from\' specifier exceeds the allowed boundaries: [%f,%f,%f]", vec.x, vec.y, vec.z);

	return vec;
}

static Direction getFacing(size_t idx) {
	Direction dir = DirectionByName(DirectionNames[idx]);

	if (dir == Direction_None)
		Crash("Unknown facing: %s (%d)", DirectionNames[idx], dir);

	return dir;
}

static void getFaces(mpack_node_t e) {
	mpack_node_t faceNodes = serial_get_node(e, "faces");

	size_t size = serial_get_mapLength(faceNodes);

	if (size > 0 || !serial_has(e, "faces")) {
		for (size_t i = 0; i < size; ++i) {
			mpack_node_t face = serial_get_node(faceNodes, DirectionNames[i]);
			Direction dir	  = getFacing(i);

			sBuffer_Faces[dir] = BlockElementFace_Deserialize(face);
			if (sBuffer_Faces[dir].cullDir == Direction_None)
				sBuffer_Faces[dir].cullDir = dir;
		}
	} else {
		Crash("Expected between 1 and 6 unique faces, got 0");
	}
}

BlockElement BlockElement_Deserialize(mpack_node_t element) {
	int3 from = parseFromPos(element);
	int3 to	  = parseToPos(element);
	getFaces(element);

	if (serial_has(element, "shade") && !(serial_is(element, "shade", bool)))
		Crash("Expected shade to be Boolean");
	bool shade = serial_get(element, bool, "shade", true);

	BlockElement obj;
	obj.from  = from;
	obj.to	  = to;
	obj.shade = shade;
	memcpy(obj.faces, sBuffer_Faces, sizeof(BlockElementFace) * 6);

	return obj;
}