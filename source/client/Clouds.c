#include "client/renderer/Clouds.h"

#include "core/VertexFmt.h"

#include <sino/sino.h>
#include <stdint.h>

#include "client/renderer/texture/TextureMap.h"

#include <stdio.h>

#define sz 40
static WorldVertex vertices[] = {
	{ { { -sz, 0, -sz } }, { 0, 0 }, { 255, 255, 255 } },
	{ { { sz, 0, -sz } }, { INT16_MAX, 0 }, { 255, 255, 255 } },
	{ { { sz, 0, sz } }, { INT16_MAX, INT16_MAX }, { 255, 255, 255 } },
	{ { { sz, 0, sz } }, { INT16_MAX, INT16_MAX }, { 255, 255, 255 } },
	{ { { -sz, 0, sz } }, { 0, INT16_MAX }, { 255, 255, 255 } },
	{ { { -sz, 0, -sz } }, { 0, 0 }, { 255, 255, 255 } },
};

static C3D_Tex texture;
static WorldVertex* cloudVBO;

#define TEXTURE_SIZE 64

void Clouds_Init() {
	u8* map = (u8*)malloc(TEXTURE_SIZE * TEXTURE_SIZE);
	for (int i = 0; i < TEXTURE_SIZE; i++) {
		for (int j = 0; j < TEXTURE_SIZE; j++) {
			float noise = sino_2d(j * 0.2f, i * 0.3f);
			for (int k = 1; k < 3; k++) {
				noise += sino_2d(j * 0.15f / 1, i * 0.2f / 1);
			}
			map[j + i * TEXTURE_SIZE] = (noise / 3.f > 0.2f) * 15 | (15 << 4);
		}
	}
	C3D_TexInit(&texture, TEXTURE_SIZE, TEXTURE_SIZE, GPU_LA4);
	C3D_TexSetWrap(&texture, GPU_REPEAT, GPU_REPEAT);
	Texture_TileImage8(map, texture.data, TEXTURE_SIZE);

	free(map);

	cloudVBO = linearAlloc(sizeof(vertices));
	memcpy(cloudVBO, vertices, sizeof(vertices));
}

void Clouds_Deinit() {
	C3D_TexDelete(&texture);
	linearFree(cloudVBO);
}

static C3D_Mtx modelMtx;

void Clouds_Tick(float tx, float ty, float tz) {
	Mtx_Identity(&modelMtx);
	Mtx_Translate(&modelMtx, tx, ty + 69.f * 16, tz, true);
	Mtx_Scale(&modelMtx, 90.f, 90.f, 90.f);

	const int stepX = 8;
	const int stepZ = 14;
	if (((int)cloudVBO[0].uv[0]) - stepX < -INT16_MAX) {
		for (int i = 0; i < 6; i++) {
			if (cloudVBO[i].pos.x == -1)
				cloudVBO[i].uv[0] = 0;
			else
				cloudVBO[i].uv[0] = INT16_MAX;
		}
	} else {
		for (int i = 0; i < 6; i++) {
			cloudVBO[i].uv[0] -= stepX;
		}
	}
	if (((int)cloudVBO[0].uv[1]) + stepZ > INT16_MAX) {
		for (int i = 0; i < 6; i++) {
			if (cloudVBO[i].pos.z == 1)
				cloudVBO[i].uv[1] = -INT16_MAX;
			else
				cloudVBO[i].uv[1] = 0;
		}
	} else {
		for (int i = 0; i < 6; i++) {
			cloudVBO[i].uv[1] += stepZ;
		}
	}
}

void Clouds_Render(int projUniform, C3D_Mtx* projectionview) {
	C3D_CullFace(GPU_CULL_NONE);

	C3D_AlphaTest(true, GPU_GREATER, 0);

	C3D_TexBind(0, &texture);

	GSPGPU_FlushDataCache(cloudVBO, sizeof(vertices));

	C3D_Mtx mvp;
	Mtx_Multiply(&mvp, projectionview, &modelMtx);

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projUniform, &mvp);

	C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, cloudVBO, sizeof(WorldVertex), 3, 0x3210);

	C3D_DrawArrays(GPU_TRIANGLES, 0, 6);

	C3D_CullFace(GPU_CULL_BACK_CCW);

	C3D_AlphaTest(false, GPU_GREATER, 0);
}
