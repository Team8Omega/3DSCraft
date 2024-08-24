#include "client/renderer/block/model/BlockElementFace.h"

#include "util/SerialUtils.h"

BlockElementFace BlockElementFace_Deserialize(mpack_node_t face) {
	char dirname[6];
	serial_get_cstr(face, "cullface", dirname, 6);
	Direction dir = DirectionByName(dirname);

	BlockElementFace obj;
	obj.cullDir	  = dir;
	obj.tintIndex = serial_get(face, int, "tintindex", -1);
	serial_get_cstr(face, "texture", obj.texture, 32);
	obj.uv = BlockFaceUV_Deserialize(face);

	return obj;
}