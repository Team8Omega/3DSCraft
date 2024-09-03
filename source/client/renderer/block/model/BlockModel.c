#include "client/renderer/block/model/BlockModel.h"

#include <3ds.h>

#include "client/renderer/block/model/BlockElement.h"
#include "util/SerialUtils.h"
#include "util/StringUtils.h"

static char parentName[64];

static BlockElement* elements	   = NULL;
static size_t elementNum		   = 0;
static ModelTextureEntry* textures = NULL;
static size_t textureNum		   = 0;

static AmbientOcc hasAmbientOcclusion = AMBIENTOCC_NONE;
static GuiLight guiLight			  = GUILIGHT_NONE;

static void getElements(mpack_node_t root) {
	if (!serial_has(root, "elements")) {
		elementNum = 0;
		return;
	}
	mpack_node_t elementNode = serial_get_node(root, "elements");

	size_t size = serial_get_arrayLength(elementNode);
	elementNum	= size;

	elements = linearAlloc(sizeof(BlockElement) * size);
	memset(elements, 0, sizeof(BlockElement) * size);

	for (size_t i = 0; i < size; ++i) {
		mpack_node_t element = serial_get_arrayNodeAt(elementNode, i);
		elements[i]			 = BlockElement_Deserialize(element);
	}
}
static void getParentName(mpack_node_t root) {
	if (!serial_has(root, "parent")) {
		parentName[0] = '\0';
		return;
	}

	serial_get_cstr(root, "parent", parentName, 64);
}
static void getTextures(mpack_node_t root) {
	if (!serial_has(root, "textures")) {
		textureNum = 0;
		return;
	}
	mpack_node_t textureNode = serial_get_node(root, "textures");

	textureNum = serial_get_mapLength(textureNode);

	textures = linearAlloc(sizeof(ModelTextureEntry) * textureNum);
	memset(textures, 0, sizeof(ModelTextureEntry) * textureNum);

	for (size_t i = 0; i < textureNum; ++i) {
		serial_get_keyname(textureNode, i, textures[i].key, 32);
		serial_get_cstr(textureNode, textures[i].key, textures[i].name, 64);
	}
}
static void getAmbientOcclusion(mpack_node_t root) {
	if (!serial_has(root, "ambientocclusion")) {
		hasAmbientOcclusion = AMBIENTOCC_NONE;
		return;
	}
	hasAmbientOcclusion = serial_get(root, bool, "ambientocclusiion", true) ? AMBIENTOCC_TRUE : AMBIENTOCC_FALSE;
}
static void getGuiLight(mpack_node_t root) {
	if (!serial_has(root, "gui_light")) {
		guiLight = GUILIGHT_NONE;
		return;
	}
	char str[6];
	serial_get_cstr(root, "gui_light", str, 6);

	guiLight = strcmp(str, "front") == 0 ? GUILIGHT_FRONT : GUILIGHT_SIDE;
}
static size_t getFaceNum() {
	size_t faceNum = 0;
	for (size_t i = 0; i < elementNum; ++i) {
		BlockElement* element = &elements[i];
		for (size_t i = 0; i < 6; ++i) {
			if (element->faces[i].exists)
				faceNum++;
		}
	}
	return faceNum;
}
BlockModel BlockModel_Deserialize(mpack_node_t root, const char* name) {
	getElements(root);
	serial_get_error(root, "Elements");

	getParentName(root);
	serial_get_error(root, "ParentName");

	getTextures(root);
	serial_get_error(root, "Textures");

	getAmbientOcclusion(root);
	serial_get_error(root, "AmbientOcculsion");

	getGuiLight(root);
	serial_get_error(root, "GuiLight");

	BlockModel obj;
	obj.hash	= String_Hash(name);
	obj.vertNum = getFaceNum() * 6;
	strcpy(obj.name, name);
	strcpy(obj.parentName, parentName);
	obj.hasAmbientOcclusion = hasAmbientOcclusion;
	obj.guiLight			= guiLight;
	obj.elementNum			= elementNum;
	if (elementNum > 0) {
		obj.elements = elements;
	} else {
		obj.elements = NULL;
	}
	obj.textureNum = textureNum;
	if (textureNum > 0) {
		obj.textures = textures;
	} else {
		obj.textures = NULL;
	}

	elements = NULL;
	textures = NULL;

	return obj;
}

int BlockModel_GetDependencies(char** out_ids[]) {
	// note: size = itemoverrideNum + (ifParent ? 1 : 0)
	bool hasParent = parentName[0] != '\0';
	int size	   = hasParent ? 1 : 0;

	if (!out_ids) {
		Crash("Segmentation Fault\nBlockModel_GetDependencies() recieved NULL ptr");
	}

	if (hasParent) {
		strncpy(*out_ids[0], parentName, 64);
	}

	/*
	if ((size - (int)hasParent) > 0) {
		// copy ItemOverrides
	}
	*/

	return size;
}