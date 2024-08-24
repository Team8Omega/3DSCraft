#include "client/renderer/block/model/BlockElement.h"

#include <mpack/mpack.h>

#include "client/renderer/block/model/BlockElementFace.h"
#include "core/Direction.h"
#include "util/SerialUtils.h"
#include "util/math/VecMath.h"

#define MIN_EXTENT -16.f
#define MAX_EXTENT 32.f

static int3 parseVector3(mpack_node_t e, const char* keyName) {
	mpack_node_t array = serial_get_node(e, keyName);
	if (serial_get_arrayLength(array) != 3)
		Crash("Expected 3 \'%s\' values, found %zu\n\'%s\' is of type %s", keyName, serial_get_arrayLength(array), keyName,
			  mpack_type_to_string(array.data->type));

	int3 val;
	for (u8 i = 0; i < 3; ++i) {
		val.v[i] = serial_get_arrayAt(array, int, i);
		Crash("type: %s", mpack_type_to_string(serial_get_typeAt(array, i)));
	}
	Crash("TEST Vec %zu %s: %f %f %f", serial_get_arrayLength(array), keyName, val.x, val.y, val.z);
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

static Direction getFacing(mpack_node_t f) {
	char dirname[6];
	serial_get_cstr(f, "direction", dirname, 6);
	Direction dir = DirectionByName(dirname);

	if (dir == Direction_None)
		Crash("Unknown facing: %u", dir);

	return dir;
}

typedef struct {
	u8 num;
	BlockElementFace* faces;
} FaceArray;

static FaceArray getFaces(mpack_node_t e) {
	mpack_node_t faceArray = serial_get_node(e, "faces");

	size_t size = serial_get_arrayLength(faceArray);

	FaceArray faces;

	if (size > 0) {
		faces.num	= size;
		faces.faces = malloc(sizeof(BlockElementFace) * size);

		for (size_t i = 0; i < size; ++i) {
			mpack_node_t face = serial_get_mapArrayAt(faceArray, i);
			Direction dir	  = getFacing(face);
			faces.faces[dir]  = BlockElementFace_Deserialize(face);
		}
	} else {
		Crash("Expected between 1 and 6 unique faces, got 0");
	}

	return faces;
}

BlockElement BlockElement_Deserialize(mpack_node_t element) {
	int3 to			= parseToPos(element);
	int3 from		= parseFromPos(element);
	FaceArray faces = getFaces(element);

	if (serial_has(element, "shade") && !(serial_is(element, "shade", bool)))
		Crash("Expected shade to be Boolean");
	bool shade = serial_get(element, bool, "shade", true);

	BlockElement obj;
	obj.from = from;
	obj.to	 = to;
	for (u8 i = 0; i < faces.num; ++i) {
		obj.faces[i] = faces.faces[i];
	}
	obj.shade = shade;

	free(faces.faces);

	return obj;
}