#include "client/renderer/block/model/BlockElementFace.h"

#include "util/SerialUtils.h"

BlockElementFace BlockElementFace_Deserialize(mpack_node_t face) {
	BlockElementFace obj;
	obj.tintIndex = serial_get(face, int, "tintindex", -1);
	serial_get_cstr(face, "texture", obj.texture, 32);
	obj.uv = BlockFaceUV_Deserialize(face);

	return obj;
}