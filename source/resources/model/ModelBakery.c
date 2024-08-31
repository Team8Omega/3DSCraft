#include "resources/model/ModelBakery.h"

#include <3ds.h>
#include <unistd.h>
#include <vec/vec.h>

#include "client/renderer/texture/TextureMap.h"
#include "util/Paths.h"
#include "util/SerialUtils.h"
#include "util/StringUtils.h"

static vec_t(BlockModel) sUnbakedBlocks;

void ModelBakery_Init() {
	vec_init(&sUnbakedBlocks);
}

void ModelBakery_Deinit() {
	BlockModel* model;
	while (sUnbakedBlocks.length > 0) {
		model = &vec_pop(&sUnbakedBlocks);

		linearFree(model->textures);
		linearFree(model->elements);
	}
}

static BlockModel loadBlockModel(const char* name) {
	const char* path = String_ParsePackName(PACK_VANILLA, PATH_PACK_MODELS, String_AddSuffix(name, ".mp"));

	BlockModel model = {};
	if (access(path, F_OK)) {
		Crash("File not found\nModelfile missing for %s\nFull Path: %s", name, path);
		return model;
	}

	mpack_tree_t levelTree = serial_get_start(path);
	mpack_node_t root	   = serial_get_root(&levelTree);
	if (serial_get_error(root, "loading BlockModel file"))
		Crash("Name: %s, Path: %s", name, path);

	model = BlockModel_Deserialize(root, name);

	return model;
}

static BlockModel* getBlockModel(const char* name) {
	u32 hashName = String_Hash(name);
	if (sUnbakedBlocks.length > 0) {
		int index;
		BlockModel* model;
		vec_foreach_ptr(&sUnbakedBlocks, model, index) {
			if (model->hash == hashName)
				return &sUnbakedBlocks.data[index];
		}
	}

	BlockModel model = loadBlockModel(name);

	BlockModel* parent = NULL;
	if (model.parentName[0] != '\0') {
		if (strncmp(model.parentName, "minecraft:", 10) == 0) {
			memmove(model.parentName, model.parentName + 10, strlen(model.parentName) - 10 + 1);
		}
		parent = getBlockModel(model.parentName);
	}

	BlockModel newModel;
	if (parent) {
		newModel	  = *(parent);
		newModel.hash = hashName;
		strcpy(newModel.name, model.name);
		strcpy(newModel.parentName, model.parentName);

		newModel.guiLight = model.guiLight != GUILIGHT_NONE ? model.guiLight : parent->guiLight;
		newModel.hasAmbientOcclusion =
			model.hasAmbientOcclusion != AMBIENTOCC_NONE ? model.hasAmbientOcclusion : parent->hasAmbientOcclusion;

		size_t texNum		= model.textureNum + parent->textureNum;
		newModel.textureNum = texNum;
		if (texNum > 0) {
			newModel.textures = linearAlloc(sizeof(ModelTextureEntry) * texNum);
			for (size_t i = 0; i < parent->textureNum; ++i) {
				memcpy(&newModel.textures[i], &parent->textures[i], sizeof(ModelTextureEntry));
			}
			for (size_t i = 0; i < model.textureNum; ++i) {
				memcpy(&newModel.textures[parent->textureNum + i], &model.textures[i], sizeof(ModelTextureEntry));
			}
		} else {
			newModel.textures = NULL;
		}

		size_t elmNum		= model.elementNum + parent->elementNum;
		newModel.elementNum = elmNum;
		if (elmNum > 0) {
			newModel.elements = linearAlloc(sizeof(BlockElement) * elmNum);
			for (size_t i = 0; i < parent->elementNum; ++i) {
				memcpy(&newModel.elements[i], &parent->elements[i], sizeof(BlockElement));
			}
			for (size_t i = 0; i < model.elementNum; ++i) {
				memcpy(&newModel.elements[parent->elementNum + i], &model.elements[i], sizeof(BlockElement));
			}
		} else {
			newModel.elements = NULL;
		}
		newModel.faceNum = model.faceNum + parent->faceNum;

	} else {
		newModel = model;
	}

	vec_push(&sUnbakedBlocks, newModel);

	return &sUnbakedBlocks.data[sUnbakedBlocks.length - 1];
}

static const bool block_lut_vertex[][3] = {
	// West
	{ false, false, false },
	{ false, false, true },
	{ false, true, true },
	{ false, true, true },
	{ false, true, false },
	{ false, false, false },
	// East
	{ true, false, false },
	{ true, true, false },
	{ true, true, true },
	{ true, true, true },
	{ true, false, true },
	{ true, false, false },
	// Down
	{ false, false, false },
	{ true, false, false },
	{ true, false, true },
	{ true, false, true },
	{ false, false, true },
	{ false, false, false },
	// Up
	{ false, true, false },
	{ false, true, true },
	{ true, true, true },
	{ true, true, true },
	{ true, true, false },
	{ false, true, false },
	// North
	{ false, false, false },
	{ false, true, false },
	{ true, true, false },
	{ true, true, false },
	{ true, false, false },
	{ false, false, false },
	// South
	{ false, false, true },
	{ true, false, true },
	{ true, true, true },
	{ true, true, true },
	{ false, true, true },
	{ false, false, true },
};
static const bool block_lut_uv[][2] = {
	// West
	{ false, false },
	{ true, false },
	{ true, true },
	{ true, true },
	{ false, true },
	{ false, false },
	// East
	{ true, false },
	{ true, true },
	{ false, true },
	{ false, true },
	{ false, false },
	{ true, false },
	// Down
	{ false, true },
	{ true, true },
	{ true, false },
	{ true, false },
	{ false, false },
	{ false, true },
	// Up
	{ false, true },
	{ false, false },
	{ true, false },
	{ true, false },
	{ true, true },
	{ false, true },
	// North
	{ true, false },
	{ true, true },
	{ false, true },
	{ false, true },
	{ false, false },
	{ true, false },
	// South
	{ false, false },
	{ true, false },
	{ true, true },
	{ true, true },
	{ false, true },
	{ false, false },
};

// name, parentname, hash, elementnum, guilight, hasambientocclusion
// elements: i3 from, to; shade
// \-faces: tintindex, texture
// uv: uvs[4], rotation

enum {
	UV_U1,
	UV_V1,
	UV_U2,
	UV_V2
};
typedef struct {
	u32 hash;
	int uv[2];
} TextureIcon;

static BakedModel* bakeBlockModel(BlockModel* blockModel) {
	BakedModel* baked = linearAlloc(sizeof(BakedModel));

	u32 currentVertex = 0;

	baked->numFaces = blockModel->faceNum;
	baked->vertex	= linearAlloc(sizeof(WorldVertex*) * 6 * 6);
	for (size_t i = 0; i < 6 * 6; ++i) {
		baked->vertex[i] = linearAlloc(sizeof(WorldVertex));
		memset(baked->vertex[i], 0, sizeof(WorldVertex));
	}

	if (!blockModel) {
		Crash("Baking BlockModel failed!\nRecieved NULL BlockModel as input.");
		return NULL;
	}

	if (((int)blockModel->elements < 100 && blockModel->elementNum != 0) ||
		((int)blockModel->textures < 100 && blockModel->textureNum != 0)) {
		Crash(
			"Baking BlockModel failed!\nRecieved invalid BlockModel as input, either elements or textures is NULL when num value isn't "
			"0\n"
			"Elements: 0x%x, Num : %lu\n"
			"Textures: 0x%x, Num : %lu\n",
			blockModel->elements, blockModel->elementNum, blockModel->textures, blockModel->textureNum);
		return NULL;
	}
	if (((int)blockModel->elements > 100 && blockModel->elementNum == 0) ||
		((int)blockModel->textures > 100 && blockModel->textureNum == 0)) {
		Crash(
			"Baking BlockModel failed!\nRecieved invalid BlockModel as input, either elements or textures doesnt line up with num "
			"value\n"
			"Elements: 0x%x, Num: %lu\n"
			"Textures: 0x%x, Num: %lu\n",
			blockModel->elements, blockModel->elementNum, blockModel->textures, blockModel->textureNum);
		return NULL;
	}

	TextureIcon textures[blockModel->textureNum];
	for (size_t i = 0; i < blockModel->textureNum; ++i) {
		textures[i].hash = String_Hash(blockModel->textures[i].key);

		if (strncmp(blockModel->textures[i].name, "minecraft:", 10) == 0) {
			memmove(blockModel->textures[i].name, blockModel->textures[i].name + 10, strlen(blockModel->textures[i].name) - 10 + 1);
		}
		Texture_MapAddName(blockModel->textures[i].name, textures[i].uv);
	}

	for (size_t i = 0; i < blockModel->elementNum; ++i) {
		BlockElement* element = &blockModel->elements[i];

		currentVertex = 0;

		float3 from = element->from, to = element->to;
		for (u8 j = 0; j < 6; ++j) {
			BlockElementFace* face = &element->faces[j];

			if (!face->exists) {
				currentVertex += 6;
				continue;
			}

			int uv[2];
			u32 hash = String_Hash(face->texture);
			for (size_t i = 0; i < blockModel->textureNum; ++i) {
				if (textures[i].hash == hash) {
					uv[0] = textures[i].uv[0];
					uv[1] = textures[i].uv[1];
				}
			}

			for (u8 k = 0; k < 6; ++k) {
				baked->vertex[currentVertex]->pos[0] = block_lut_vertex[currentVertex][0] ? to.x : from.x;
				baked->vertex[currentVertex]->pos[1] = block_lut_vertex[currentVertex][1] ? to.y : from.y;
				baked->vertex[currentVertex]->pos[2] = block_lut_vertex[currentVertex][2] ? to.z : from.z;

				baked->vertex[currentVertex]->uv0[0] = (block_lut_uv[currentVertex][0] ? face->uv.uvs[UV_U2] : face->uv.uvs[UV_U1]) + uv[0];
				baked->vertex[currentVertex]->uv0[1] = (block_lut_uv[currentVertex][1] ? face->uv.uvs[UV_V2] : face->uv.uvs[UV_V1]) + uv[1];

				baked->vertex[currentVertex]->rgb[0] = 255;
				baked->vertex[currentVertex]->rgb[1] = 255;
				baked->vertex[currentVertex]->rgb[2] = 255;

				currentVertex++;
			}
		}
	}

	return baked;
}

BakedModel* ModelBakery_GetModel(const char* name) {
	// u32 startTime = svcGetSystemTick();

	BlockModel* unbaked = getBlockModel(name);

	/*
	u32 endTime		  = svcGetSystemTick();
	float elapsedMs	  = (float)(endTime - startTime) / (float)CPU_TICKS_PER_MSEC;
	const char* light = unbaked->guiLight != GUILIGHT_NONE ? unbaked->guiLight == GUILIGHT_FRONT ? "front" : "side" : "none";
	Crash("Loaded Model! Took %.3f ms.\n\nName: %s,\nParent: %s,\nElementNum: %lu\nGuiLight: %s\nTextureNum: %d\nLinearFree: %lu KB",
		  elapsedMs, unbaked->name, unbaked->parentName, unbaked->elementNum, light, unbaked->textureNum, linearSpaceFree() >> 10);
	*/
	return bakeBlockModel(unbaked);
}