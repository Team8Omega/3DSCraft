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

static BlockModel loadBlockModel(const char* name) {
	const char* path = String_ParsePackName(PACK_VANILLA, PATH_PACK_MODELS, String_AddSuffix(name, ".mp"));

	BlockModel model = {};
	if (access(path, F_OK) == -1) {
		Crash("File not found\nModelfile missing for %s\n", name);
		return model;
	}

	mpack_tree_t levelTree = serial_get_start(path);
	model				   = BlockModel_Deserialize(serial_get_root(&levelTree), name);

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

		if (model.elementNum > 0) {
			newModel.elementNum = model.elementNum;
			newModel.elements	= model.elements;
		} else {
			newModel.elementNum = parent->elementNum;
			newModel.elements	= model.elements;
		}

		size_t newTexNum  = model.textureNum + parent->textureNum;
		newModel.textures = malloc(sizeof(ModelTextureEntry) * newTexNum);
		for (size_t i = 0; i < parent->textureNum; ++i) {
			newModel.textures[i] = parent->textures[i];
		}
		for (size_t i = 0; i < model.textureNum; ++i) {
			newModel.textures[parent->textureNum + i] = model.textures[i];
		}
	} else {
		newModel = model;
	}

	vec_push(&sUnbakedBlocks, newModel);

	return &sUnbakedBlocks.data[sUnbakedBlocks.length - 1];
}

static const WorldVertex block_sides_lut[] = {
	// West
	{ { 0, 0, 0 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { 0, 0, 1 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 0, 1, 1 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 0, 1, 1 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 0, 1, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 0, 0, 0 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	// East
	{ { 1, 0, 0 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 1, 1, 0 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 1, 1, 1 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 1, 1, 1 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 1, 0, 1 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { 1, 0, 0 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	// Down
	{ { 0, 0, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 1, 0, 0 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 1, 0, 1 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 1, 0, 1 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 0, 0, 1 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { 0, 0, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	// Up
	{ { 0, 1, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 0, 1, 1 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { 1, 1, 1 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 1, 1, 1 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 1, 1, 0 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 0, 1, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	// North
	{ { 0, 0, 0 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 0, 1, 0 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 1, 1, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 1, 1, 0 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 1, 0, 0 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { 0, 0, 0 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	// South
	{ { 0, 0, 1 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { 1, 0, 1 }, { 1, 0 }, { 1, 0 }, { 255, 255, 255 } },
	{ { 1, 1, 1 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 1, 1, 1 }, { 1, 1 }, { 1, 1 }, { 255, 255, 255 } },
	{ { 0, 1, 1 }, { 0, 1 }, { 0, 1 }, { 255, 255, 255 } },
	{ { 0, 0, 1 }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
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
	u32 currentFace	  = 0;

	baked->faces	= linearAlloc(sizeof(WorldVertex) * blockModel->faceNum * 6);
	baked->numFaces = blockModel->faceNum;

	TextureIcon textures[blockModel->textureNum];
	for (size_t i = 0; i < blockModel->textureNum; ++i) {
		textures[i].hash = String_Hash(blockModel->textures[i].key);
		Texture_MapAddName(blockModel->textures[i].name, textures[i].uv);
	}

	for (size_t i = 0; i < blockModel->elementNum; ++i) {
		BlockElement* element = &blockModel->elements[i];

		int3 from = element->from, to = element->to;
		for (u8 j = 0; j < 6; ++j) {
			BlockElementFace* face = &element->faces[j];

			int uv[2];
			u32 hash = String_Hash(face->texture);
			for (size_t i = 0; i < blockModel->textureNum; ++i) {
				if (textures[i].hash == hash) {
					uv[0] = textures[i].uv[0];
					uv[1] = textures[i].uv[1];
				}
			}

			if (!face->exists)
				continue;

			WorldVertex** vertices = &baked->faces[6 * currentFace++];
			memcpy(vertices, &block_sides_lut[6 * j], sizeof(WorldVertex) * 6);
			for (u8 k = 0; k < 6; ++k) {
				vertices[k]->pos[0] = vertices[k]->pos[0] == 0 ? from.x : to.x;
				vertices[k]->pos[1] = vertices[k]->pos[1] == 0 ? from.y : to.y;
				vertices[k]->pos[2] = vertices[k]->pos[2] == 0 ? from.z : to.z;

				vertices[k]->uv0[0] = (vertices[k]->uv0[0] ? face->uv.uvs[UV_U1] : face->uv.uvs[UV_U2] + uv[0]);
				vertices[k]->uv0[1] = (vertices[k]->uv0[1] ? face->uv.uvs[UV_V1] : face->uv.uvs[UV_V2] + uv[1]);
			}
		}
	}

	return baked;
}  // note: free elements and textures sometime later, let this stay as is for now. once all blocks baked, cleanup.

BakedModel* ModelBakery_GetModel(const char* name) {
	u32 startTime = svcGetSystemTick();

	BlockModel* unbaked = getBlockModel(name);

	u32 endTime		  = svcGetSystemTick();
	float elapsedMs	  = (float)(endTime - startTime) / (float)CPU_TICKS_PER_MSEC;
	const char* light = unbaked->guiLight != GUILIGHT_NONE ? unbaked->guiLight == GUILIGHT_FRONT ? "front" : "side" : "none";
	Crash(
		"Loaded Model! Took %.3f ms.\n\nName: %s,\nParent: %s,\nElementNum: %lu\nGuiLight: %s\nTextures: (%lu) %s:%s %s:%s %s:%s %s:%s "
		"%s:%s",
		elapsedMs, unbaked->name, unbaked->parentName, unbaked->elementNum, light, unbaked->textureNum, unbaked->textures[0].key,
		unbaked->textures[0].name, unbaked->textures[1].key, unbaked->textures[1].name, unbaked->textures[2].key, unbaked->textures[2].name,
		unbaked->textures[3].key, unbaked->textures[3].name, unbaked->textures[4].key, unbaked->textures[4].name);

	return bakeBlockModel(unbaked);
}