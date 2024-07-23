#include "client/renderer/CubeMap.h"

#include "client/Crash.h"
#include "client/model/VertexFmt.h"
#include "client/renderer/texture/TextureMap.h"
#include "util/Paths.h"
#include "util/math/NumberUtils.h"

#include <stdio.h>

#define vSize 1
#define vUV 1

const WorldVertex cube_center_lut[6 * 6] = {
	// Front face
	{ { -vSize, -vSize, vSize }, { 0, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, -vSize, vSize }, { vUV, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, vSize, vSize }, { vUV, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },

	{ { -vSize, -vSize, vSize }, { 0, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, vSize, vSize }, { vUV, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { -vSize, vSize, vSize }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },

	// Back face
	{ { -vSize, -vSize, -vSize }, { 0, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, -vSize, -vSize }, { vUV, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, vSize, -vSize }, { vUV, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },

	{ { -vSize, -vSize, -vSize }, { 0, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, vSize, -vSize }, { vUV, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { -vSize, vSize, -vSize }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },

	// Left face
	{ { -vSize, -vSize, -vSize }, { 0, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { -vSize, -vSize, vSize }, { vUV, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { -vSize, vSize, vSize }, { vUV, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },

	{ { -vSize, -vSize, -vSize }, { 0, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { -vSize, vSize, vSize }, { vUV, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { -vSize, vSize, -vSize }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },

	// Right face
	{ { vSize, -vSize, -vSize }, { 0, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, -vSize, vSize }, { vUV, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, vSize, vSize }, { vUV, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },

	{ { vSize, -vSize, -vSize }, { 0, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, vSize, vSize }, { vUV, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, vSize, -vSize }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },

	// Top face
	{ { -vSize, vSize, -vSize }, { 0, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, vSize, -vSize }, { vUV, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, vSize, vSize }, { vUV, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },

	{ { -vSize, vSize, -vSize }, { 0, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, vSize, vSize }, { vUV, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { -vSize, vSize, vSize }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },

	// Bottom face
	{ { -vSize, -vSize, -vSize }, { 0, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, -vSize, -vSize }, { vUV, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, -vSize, vSize }, { vUV, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },

	{ { -vSize, -vSize, -vSize }, { 0, vUV }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { vSize, -vSize, vSize }, { vUV, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { -vSize, -vSize, vSize }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
};

static int projUniform;
static WorldVertex* cubeVBO;
static C3D_Tex cubeTextures[6];
static float3 rotation;

void CubeMap_Init(int projUniform_) {
	projUniform = projUniform_;
	cubeVBO		= linearAlloc(sizeof(cube_center_lut));
}

void CubeMap_Set(const char* filename, float3 rotation_) {
	rotation = rotation_;

	char outName[60];
	for (u8 i = 0; i < 6; i++) {
		snprintf(outName, sizeof(outName), "%s_%i.png", filename, i);
		Texture_Load(&cubeTextures[i], outName);
		C3D_TexSetFilter(&cubeTextures[i], GPU_LINEAR, GPU_LINEAR);
		C3D_TexSetWrap(&cubeTextures[i], GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
	}
}

void CubeMap_Deinit() {
	linearFree(cubeVBO);
	for (u8 i = 0; i < 6; i++) {
		C3D_TexDelete(&cubeTextures[i]);
	}
}

void CubeMap_Draw(C3D_Mtx* projection, float3 rotationOffset) {
	if (cubeVBO == NULL) {
		Crash("CubeMap not set!");
		return;
	}

	GSPGPU_FlushDataCache(cubeVBO, sizeof(cube_center_lut));

	C3D_Mtx model, out;
	Mtx_Identity(&model);
	Mtx_Translate(&model, 0, 0, -1.f, true);

	rotation = f3_add(rotation, rotationOffset);
	Mtx_RotateX(&model, rotation.x, true);
	Mtx_RotateY(&model, rotation.y, true);
	Mtx_RotateZ(&model, rotation.z, true);

	Mtx_Multiply(&out, projection, &model);

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projUniform, &out);

	memcpy(cubeVBO, cube_center_lut, sizeof(cube_center_lut));

	C3D_CullFace(GPU_CULL_NONE);

	C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, cubeVBO, sizeof(WorldVertex), 4, 0x3210);

	for (u8 i = 0; i < 6; i++) {
		C3D_TexBind(0, &cubeTextures[i]);
		C3D_DrawArrays(GPU_TRIANGLES, i * 6, 6);
	}

	C3D_CullFace(GPU_CULL_BACK_CCW);
}
