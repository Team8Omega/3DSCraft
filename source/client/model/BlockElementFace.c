#include "client/model/BlockElementFace.h"

#include "util/SerialUtils.h"

BlockElementFace BlockElementFace_Deserialize(mpack_node_t face) {
	char dirname[6];
	serial_get_cstr(face, "cullface", dirname);
	Direction dir = DirectionByName(dirname);

	BlockElementFace obj;
	obj.cullDir	  = dir;
	obj.tintIndex = serial_get(face, int, "tintindex", -1);
	serial_get_cstr(face, "texture", obj.texture);
	obj.uv = BlockFaceUV_Deserialize(face);

	return obj;
}