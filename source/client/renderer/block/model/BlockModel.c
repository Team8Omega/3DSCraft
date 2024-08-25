#include "client/renderer/block/model/BlockModel.h"

#include "client/renderer/block/model/BlockElement.h"

#include "util/SerialUtils.h"

static u8 elementNum;
static BlockElement* elements;

#define TEXTURE_NUM_MAX 16
static char textures[TEXTURE_NUM_MAX][64];
static char textureKeys[TEXTURE_NUM_MAX][32];
static size_t textureNum		= 0;
static bool hasAmbientOcclusion = true;

static char name[64];
static char parentName[64];

static void getElements(mpack_node_t root) {
	if (!serial_has(root, "elements")) {
		elementNum = 0;
		return;
	}
	mpack_node_t elementNode = serial_get_node(root, "elements");

	size_t size = serial_get_arrayLength(elementNode);
	elements	= malloc(sizeof(BlockElement) * size);

	for (size_t i = 0; i < size; ++i) {
		mpack_node_t element = serial_get_arrayNodeAt(elementNode, i);
		elements[i]			 = BlockElement_Deserialize(element);
	}
}
static void getParentName(mpack_node_t root) {
	if (!serial_has(root, "parent")) {
		strcpy(parentName, "");
		return;
	}

	serial_get_cstr(root, "parent", parentName, 64);
}
static void getTextures(mpack_node_t root) {
	if (!serial_has(root, "textures")) {
		strcpy(textures[0], "");
		return;
	}
	mpack_node_t textureNode = serial_get_node(root, "textures");

	textureNum = serial_get_mapLength(textureNode);
	for (size_t i = 0; i < textureNum; ++i) {
		serial_get_keyname(textureNode, i, textureKeys[i], 32);
		serial_get_cstr(textureNode, textureKeys[i], textures[i], 64);
	}
}
static void getAmbientOcclusion(mpack_node_t root) {
	if (!serial_has(root, "ambientocclusion")) {
		hasAmbientOcclusion = true;
		return;
	}
	hasAmbientOcclusion = serial_get(root, bool, "ambientocclusiion", true);
}
BlockModel BlockModel_Deserialize(mpack_node_t root) {
	getElements(root);
	serial_get_error(root, "Elements");

	getParentName(root);
	serial_get_error(root, "ParentName");

	getTextures(root);
	serial_get_error(root, "Textures");

	getAmbientOcclusion(root);
	serial_get_error(root, "AmbientOcculsion");

	BlockModel obj;
	obj.name = "bob";

	return obj;
}