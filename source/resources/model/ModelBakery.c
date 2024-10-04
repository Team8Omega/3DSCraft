#include "resources/model/ModelBakery.h"

#include <3ds.h>
#include <unistd.h>
#include <vec/vec.h>

#include "client/renderer/texture/TextureMap.h"
#include "util/Paths.h"
#include "util/SerialUtils.h"
#include "util/StringUtils.h"

typedef struct {
	BlockElement* elements;
	size_t elementNum;
} ElementBuffer;

static vec_t(BlockModel) sUnbakedBlocks;
extern u32 __ctru_linear_heap_size;

void ModelBakery_Init() {
	vec_init(&sUnbakedBlocks);
}

void ModelBakery_Deinit() {
	BlockModel* model;
	while (sUnbakedBlocks.length > 0) {
		model = &vec_pop(&sUnbakedBlocks);

		if (model->textures && model->textureNum > 1)
			free(model->textures);
		if (model->elements && model->elementNum > 1)
			free(model->elements);
	}
}

static BlockModel loadBlockModel(const char* name) {
	const char* path = String_ParsePackName(PACK_VANILLA, PATH_PACK_MODELS, String_AddSuffix(name, ".mp"));

	BlockModel model = {};
	if (access(path, F_OK)) {
		Crash(0, "File not found\nModelfile missing for %s\nFull Path: %s", name, path);
		return model;
	}

	mpack_tree_t levelTree = serial_get_start(path);
	mpack_node_t root	   = serial_get_root(&levelTree);

	int err = serial_get_errorTree(&levelTree, 0, false);
	if (err) {
		Crash(CRASH_ALLOC, "MPERR %d: \"%s\"\nBlockModel File could not be loaded:\n%s", err, mpack_error_to_string(err), path);
		return model;
	}

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
		namePrefixMC(model.parentName);
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
			newModel.textures = malloc(sizeof(ModelTextureEntry) * texNum);
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
			newModel.elements = malloc(sizeof(BlockElement) * elmNum);
			for (size_t i = 0; i < parent->elementNum; ++i) {
				memcpy(&newModel.elements[i], &parent->elements[i], sizeof(BlockElement));
			}
			for (size_t i = 0; i < model.elementNum; ++i) {
				memcpy(&newModel.elements[parent->elementNum + i], &model.elements[i], sizeof(BlockElement));
			}
		} else {
			newModel.elements = NULL;
		}

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
	u32 refHash;
	u32 hash;
	int uv[2];
} TextureIcon;

static BakedModel* bakeBlockModel(BlockModel* blockModel) {
	u32 currentVertex = 0;

	BakedModel* baked = malloc(sizeof(BakedModel));

	baked->numVertex = blockModel->elementNum * (6 * 6);
	baked->vertex	 = malloc(sizeof(WorldVertex) * baked->numVertex);

	if (baked->vertex == NULL) {
		Crash(CRASH_ALLOC, "Allocation Error for BlockModel Baking\nName: %s", blockModel->name);
		free(baked);
		return NULL;
	}

	memset(baked->vertex, 0, sizeof(WorldVertex) * baked->numVertex);

	/*if (((int)blockModel->elements < 100 && blockModel->elementNum != 0) ||
		((int)blockModel->textures < 100 && blockModel->textureNum != 0)) {
		Crash(0,
			"Baking BlockModel failed!\nRecieved invalid BlockModel as input, either elements or textures is NULL when num value isn't "
			"0\n"
			"Elements: 0x%x, Num : %lu\n"
			"Textures: 0x%x, Num : %lu\n",
			blockModel->elements, blockModel->elementNum, blockModel->textures, blockModel->textureNum);
		return NULL;
	}
	if (((int)blockModel->elements > 100 && blockModel->elementNum == 0) ||
		((int)blockModel->textures > 100 && blockModel->textureNum == 0)) {
		Crash(0,
			"Baking BlockModel failed!\nRecieved invalid BlockModel as input, either elements or textures doesnt line up with num "
			"value\n"
			"Elements: 0x%x, Num: %lu\n"
			"Textures: 0x%x, Num: %lu\n",
			blockModel->elements, blockModel->elementNum, blockModel->textures, blockModel->textureNum);
		return NULL;
	}*/

	TextureIcon textures[blockModel->textureNum];

	for (size_t i = 0; i < blockModel->textureNum; ++i) {
		const char* name = blockModel->textures[i].name;

		textures[i].hash  = String_Hash(blockModel->textures[i].key);
		textures[i].uv[0] = 0;
		textures[i].uv[1] = 0;

		if (strncmp(blockModel->textures[i].name, "#", 1) != 0) {
			if (strncmp(blockModel->textures[i].name, "minecraft:", 10) == 0)
				name += 10;

			Texture_MapAddName(name, textures[i].uv);

			textures[i].refHash = 0;
		} else {
			const char* refName = name;
			refName++;

			textures[i].refHash = String_Hash(refName);
		}
	}
	for (size_t i = 0; i < blockModel->textureNum; ++i) {
		if (textures[i].refHash) {
			for (size_t j = 0; j < blockModel->textureNum; ++j) {
				if (textures[j].hash == textures[i].refHash) {
					textures[i].uv[0] = textures[j].uv[0];
					textures[i].uv[1] = textures[j].uv[1];
				}
			}
			if (textures[i].uv[0] == 0 && textures[i].uv[1] == 0)
				Crash(0, "Could not get texture reference!\nBlock: %s\nTextureIdx: %d", blockModel->name, i);
		}
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

			char* texName = face->texture;
			texName++;

			u32 hash  = String_Hash(texName);
			int uv[2] = { 0, 0 };
			for (size_t i = 0; i < blockModel->textureNum; ++i) {
				if (textures[i].hash == hash) {
					uv[0] = textures[i].uv[0];
					uv[1] = textures[i].uv[1];
				}
			}

			if (uv[0] == 0 && uv[1] == 0)
				Crash(0, "Could not get texture reference from face!\nBlock: %s\nTexture: %s\nAttempted search: %s", blockModel->name,
					  face->texture, texName);

			for (u8 k = 0; k < 6; ++k) {
				baked->vertex[currentVertex].pos.x = block_lut_vertex[currentVertex][0] ? to.x : from.x;
				baked->vertex[currentVertex].pos.y = block_lut_vertex[currentVertex][1] ? to.y : from.y;
				baked->vertex[currentVertex].pos.z = block_lut_vertex[currentVertex][2] ? to.z : from.z;

#define toTexCrd(x, tw) (s16)(((float)(x) / (float)(tw)) * (float)((1 << 15) - 1))

				baked->vertex[currentVertex].uv[0] =
					toTexCrd(uv[0] + (block_lut_uv[currentVertex][0] ? face->uv.uvs[UV_U2] : face->uv.uvs[UV_U1]), TEXTURE_MAPSIZE);
				baked->vertex[currentVertex].uv[1] =
					toTexCrd(uv[1] + (block_lut_uv[currentVertex][1] ? face->uv.uvs[UV_V2] : face->uv.uvs[UV_V1]), TEXTURE_MAPSIZE);

				baked->vertex[currentVertex].rgb[0] = 255;
				baked->vertex[currentVertex].rgb[1] = 255;
				baked->vertex[currentVertex].rgb[2] = 255;

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
	Crash(0, "Loaded Model! Took %.3f ms.\n\nName: %s,\nParent: %s,\nElementNum: %lu\nGuiLight: %s\nTextureNum: %d\nLinearFree: %lu KB",
		  elapsedMs, unbaked->name, unbaked->parentName, unbaked->elementNum, light, unbaked->textureNum, linearSpaceFree() >> 10);
	*/
	return bakeBlockModel(unbaked);
}