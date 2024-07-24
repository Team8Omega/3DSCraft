#include "client/model/Cube.h"

#include "client/Crash.h"
#include "util/math/NumberUtils.h"

extern const WorldVertex cube_sides_lut[CUBE_VERTICE_NUM];

static WorldVertex* vbo;

Cube* Cube_Init(CubeRaw* in) {
	if (!in) {
		Crash("Passed unbaked CubeRaw is NULL!");
		return NULL;
	}

	Cube* cube = linearAlloc(sizeof(Cube));
	if (!cube) {
		Crash("Could not allocate memory for cube");
		return NULL;
	}

	if (!vbo)
		vbo = linearAlloc(sizeof(cube_sides_lut));

	for (u8 face = 0; face < 6; ++face) {
		u8 lutStartIndex = face * 6;

		// Apply transformations for each vertex in the LUT
		for (int i = 0; i < 6; ++i) {
			u8 idx				= lutStartIndex + i;
			WorldVertex* vertex = &cube->vertices[idx];

			vertex->pos[0] = in->from[0] + (in->to[0] - in->from[0]) * cube_sides_lut[idx].pos[0];
			vertex->pos[1] = in->from[1] + (in->to[1] - in->from[1]) * cube_sides_lut[idx].pos[1];
			vertex->pos[2] = in->from[2] + (in->to[2] - in->from[2]) * cube_sides_lut[idx].pos[2];

#define toTexCrd(x, tw) (s16)(((float)(x) / (float)(tw)) * (float)(1 << 15))

			vertex->uv[0] = toTexCrd(in->faceUV[face][cube_sides_lut[idx].uv[0] * 2], in->dimensions[0]);
			if (vertex->uv[0] < 0)
				vertex->uv[0] = (1 << 15) - 1;
			vertex->uv[1] = toTexCrd(in->faceUV[face][cube_sides_lut[idx].uv[1] * 2 + 1], in->dimensions[1]);
			if (vertex->uv[1] < 0)
				vertex->uv[1] = (1 << 15) - 1;

			// (for simplicity, using white here)
			vertex->rgb[0] = 255;
			vertex->rgb[1] = 255;
			vertex->rgb[2] = 255;

			vertex->fxyz[0] = 0;
			vertex->fxyz[1] = 0;
			vertex->fxyz[2] = 0;
		}
	}
	C3D_Mtx matrix;
	Mtx_Identity(&matrix);
	Mtx_Translate(&matrix, in->position[0], in->position[1], in->position[2], true);
	Mtx_RotateX(&matrix, in->rotation[0], true);
	Mtx_RotateY(&matrix, in->rotation[1], true);
	Mtx_RotateZ(&matrix, in->rotation[2], true);

	Mtx_Copy(&cube->localMatrix, &matrix);
	Mtx_Copy(&cube->initialMatrix, &matrix);

	return cube;
}

void Cube_Deinit(Cube* cube) {
	if (cube == NULL)
		return;

	linearFree(cube->vertices);
	linearFree(cube);
}

void Cube_Draw(Cube* cube, int shaderUniform, C3D_Mtx* matrix) {
	if (cube == NULL) {
		Crash("Cube is NULL!");
		return;
	}

	if (!vbo)
		vbo = linearAlloc(sizeof(cube_sides_lut));

	GSPGPU_FlushDataCache(vbo, sizeof(cube_sides_lut));

	memcpy(vbo, cube->vertices, sizeof(cube_sides_lut));

	C3D_Mtx outMatrix;
	Mtx_Identity(&outMatrix);
	Mtx_Multiply(&outMatrix, matrix, &cube->localMatrix);

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, shaderUniform, &outMatrix);

	C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, vbo, sizeof(WorldVertex), 4, 0x3210);

	C3D_DrawArrays(GPU_TRIANGLES, 0, CUBE_VERTICE_NUM);
}

void Cube_Reset(Cube* c) {
	Mtx_Identity(&c->localMatrix);
}
void Cube_ResetToInit(Cube* c) {
	Cube_Reset(c);
	Mtx_Copy(&c->localMatrix, &c->initialMatrix);
}
void Cube_SetPos(Cube* cube, float3 pos) {
	Mtx_Translate(&cube->localMatrix, pos.x, pos.y, pos.z, true);
}
void Cube_SetRot(Cube* c, float3 rot) {
	Mtx_RotateX(&c->localMatrix, rot.x, true);
	Mtx_RotateY(&c->localMatrix, rot.y, true);
	Mtx_RotateZ(&c->localMatrix, rot.z, true);
}