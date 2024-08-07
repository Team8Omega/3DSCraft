#include "client/renderer/CubeMap.h"

#include "client/Crash.h"
#include "client/model/VertexFmt.h"
#include "client/renderer/texture/TextureMap.h"
#include "util/Paths.h"
#include "util/math/NumberUtils.h"

#include <stdio.h>

#define vSize 3
#define vUV ((1 << 15) - 1)
const WorldVertex vertices[6 * 6] = {
	// Face 1
	{ { -vSize, -vSize, vSize }, { vUV, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, -vSize, vSize }, { 0, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, vSize, vSize }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, -vSize, vSize }, { vUV, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, vSize, vSize }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, vSize, vSize }, { vUV, 0 }, { 0, 0 }, { 255, 255, 255 } },

	// Face 2
	{ { -vSize, -vSize, -vSize }, { vUV, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, -vSize, vSize }, { 0, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, vSize, vSize }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, -vSize, -vSize }, { vUV, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, vSize, vSize }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, vSize, -vSize }, { vUV, 0 }, { 0, 0 }, { 255, 255, 255 } },

	// Face 3
	{ { vSize, -vSize, -vSize }, { vUV, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, -vSize, -vSize }, { 0, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, vSize, -vSize }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, -vSize, -vSize }, { vUV, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, vSize, -vSize }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, vSize, -vSize }, { vUV, 0 }, { 0, 0 }, { 255, 255, 255 } },

	// Face 4
	{ { vSize, -vSize, vSize }, { vUV, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, -vSize, -vSize }, { 0, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, vSize, -vSize }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, -vSize, vSize }, { vUV, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, vSize, -vSize }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, vSize, vSize }, { vUV, 0 }, { 0, 0 }, { 255, 255, 255 } },

	// Face 5
	{ { -vSize, vSize, vSize }, { vUV, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, vSize, vSize }, { 0, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, vSize, -vSize }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, vSize, vSize }, { vUV, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, vSize, -vSize }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, vSize, -vSize }, { vUV, 0 }, { 0, 0 }, { 255, 255, 255 } },

	// Face 6
	{ { -vSize, -vSize, -vSize }, { vUV, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, -vSize, -vSize }, { 0, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, -vSize, vSize }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, -vSize, -vSize }, { vUV, vUV }, { 0, 0 }, { 255, 255, 255 } },
	{ { vSize, -vSize, vSize }, { 0, 0 }, { 0, 0 }, { 255, 255, 255 } },
	{ { -vSize, -vSize, vSize }, { vUV, 0 }, { 0, 0 }, { 255, 255, 255 } },
};

static int projUniform;
static WorldVertex* cubeVBO;
static C3D_Tex cubeTextures[6];
static float3 rotation;
static C3D_Mtx cubeMatrix;

void CubeMap_Init(int projUniform_) {
	projUniform = projUniform_;
	cubeVBO		= linearAlloc(sizeof(vertices));
}

void CubeMap_Set(const char* filename, float3 rotation_) {
	rotation = rotation_;

	char outName[60];
	for (u8 i = 0; i < 6; i++) {
		snprintf(outName, sizeof(outName), "%s_%i.png", filename, i);
		Texture_Load(&cubeTextures[i], outName);
#define TEX_FILTER GPU_LINEAR
		C3D_TexSetFilter(&cubeTextures[i], TEX_FILTER, TEX_FILTER);
		C3D_TexSetWrap(&cubeTextures[i], GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
	}
}

void CubeMap_Deinit() {
	linearFree(cubeVBO);
	for (u8 i = 0; i < 6; i++) {
		C3D_TexDelete(&cubeTextures[i]);
	}
}

void CubeMap_Update(C3D_Mtx* projection, float3 rotationOffset) {
	if (cubeVBO == NULL) {
		Crash("CubeMap not set!");
		return;
	}

	C3D_Mtx model;
	Mtx_PerspTilt(projection, C3D_AngleFromDegrees(80.0f), C3D_AspectRatioTop, 0.01f, 1000.0f, false);
	Mtx_Identity(&model);
	Mtx_Translate(&model, 0.f, 0.f, 0.f, true);

	rotation = f3_add(rotation, rotationOffset);
	Mtx_RotateX(&model, rotation.x, true);
	Mtx_RotateY(&model, rotation.y, true);
	Mtx_RotateZ(&model, rotation.z, true);

	Mtx_Multiply(&cubeMatrix, projection, &model);
}
void CubeMap_Draw() {
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projUniform, &cubeMatrix);

	C3D_CullFace(GPU_CULL_FRONT_CCW);

	memcpy(cubeVBO, vertices, sizeof(vertices));

	GSPGPU_FlushDataCache(cubeVBO, sizeof(vertices));

	C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, cubeVBO, sizeof(WorldVertex), 4, 0x3210);

	// ToDo: find Alternative method to disable fog
	C3D_FogGasMode(false, 0, false);

	for (u8 i = 0; i < 6; i++) {
		C3D_TexBind(0, &cubeTextures[i]);
		C3D_DrawArrays(GPU_TRIANGLES, i * 6, 6);
	}

	C3D_CullFace(GPU_CULL_BACK_CCW);
}
