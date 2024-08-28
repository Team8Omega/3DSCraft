#include "client/renderer/block/model/BlockFaceUV.h"

#include "util/SerialUtils.h"

u16 sBuffer_UV[4] = { 0 };

static void getUVs(mpack_node_t face) {
	if (!serial_has(face, "uv")) {
		memset(sBuffer_UV, 0, sizeof(sBuffer_UV));
		return;
	}
	mpack_node_t uvNode = serial_get_node(face, "uv");

	if (serial_get_arrayLength(uvNode) != 4) {
		Crash("Expected 4 uv values, found: %d", serial_get_arrayLength(uvNode));
		memset(sBuffer_UV, 0, sizeof(sBuffer_UV));
		return;
	}

	for (u8 i = 0; i < 4; ++i) {
		sBuffer_UV[i] = (u16)serial_get_at(uvNode, int, i);
	}
}
static int getRotation(mpack_node_t face) {
	int rot = serial_get(face, int, "rotation", 0);
	if (rot != 0 && (rot < 0 || (rot % 90) != 0 || (rot / 90) > 3))
		Crash("Invalid rotation %d found, only 0/90/180/270 allowed");

	return rot;
}

BlockFaceUV BlockFaceUV_Deserialize(mpack_node_t face) {
	getUVs(face);
	int rot = getRotation(face);
	BlockFaceUV obj;
	memcpy(obj.uvs, sBuffer_UV, sizeof(sBuffer_UV));
	obj.rotation = rot;
	return obj;
}