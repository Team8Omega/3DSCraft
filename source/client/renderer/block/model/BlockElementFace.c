#include "client/renderer/block/model/BlockElementFace.h"

#include "util/SerialUtils.h"

static Direction inline getCullface(mpack_node_t face) {
	if (!serial_has(face, "cullface"))
		return Direction_None;

	char dirname[12];
	serial_get_cstr(face, "cullface", dirname, 12);
	return DirectionByName(dirname);
}

BlockElementFace BlockElementFace_Deserialize(mpack_node_t face) {
	BlockElementFace obj;
	obj.cullDir	  = getCullface(face);
	obj.tintIndex = serial_get(face, int, "tintindex", -1);
	serial_get_cstr(face, "texture", obj.texture, 32);
	obj.uv = BlockFaceUV_Deserialize(face);

	return obj;
}