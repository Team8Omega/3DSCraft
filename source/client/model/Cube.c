#include "client/model/Cube.h"

#include "vec/vec.h"

#include "client/Crash.h"
#include "core/Direction.h"
#include "util/math/NumberUtils.h"

#define CUBE_NUM_MAX 64

static u16 cubeNum = 0;
static Cube* cubeRef[CUBE_NUM_MAX];
static WorldVertex* cubeModelVBOs[CUBE_NUM_MAX];

const WorldVertex cube_sides_lut[] = {
	// Fourth face (MX) - West
	// First triangle
	{ { 0, 0, 0 }, { 0, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 0, 0, 1 }, { 1, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 0, 1, 1 }, { 1, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	// Second triangle
	{ { 0, 1, 1 }, { 1, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 0, 1, 0 }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 0, 0, 0 }, { 0, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	// Third face (PX) - East
	// First triangle
	{ { 1, 0, 0 }, { 0, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 1, 1, 0 }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 1, 1, 1 }, { 1, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	// Second triangle
	{ { 1, 1, 1 }, { 1, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 1, 0, 1 }, { 1, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 1, 0, 0 }, { 0, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	// Sixth face (MY) - Down
	// First triangle
	{ { 0, 0, 0 }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 1, 0, 0 }, { 1, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 1, 0, 1 }, { 1, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	// Second triangle
	{ { 1, 0, 1 }, { 1, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 0, 0, 1 }, { 0, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 0, 0, 0 }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	// Fifth face (PY) - Up
	// First triangle
	{ { 0, 1, 0 }, { 0, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 0, 1, 1 }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 1, 1, 1 }, { 1, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	// Second triangle
	{ { 1, 1, 1 }, { 1, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 1, 1, 0 }, { 1, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 0, 1, 0 }, { 0, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	// Second face (MZ) - North
	// First triangle
	{ { 0, 0, 1 }, { 1, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 1, 0, 1 }, { 0, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 1, 1, 1 }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	// Second triangle
	{ { 1, 1, 1 }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 0, 1, 1 }, { 1, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 0, 0, 1 }, { 1, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	// First face (PZ) - South
	// First triangle
	{ { 0, 0, 0 }, { 1, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 1, 0, 0 }, { 0, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 1, 1, 0 }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	// Second triangle
	{ { 1, 1, 0 }, { 0, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 0, 1, 0 }, { 1, 0 }, { 255, 255, 255 }, { 0, 0, 0 } },
	{ { 0, 0, 0 }, { 1, 1 }, { 255, 255, 255 }, { 0, 0, 0 } },
};

void Cube_InitVBOs() {
	for (u16 i = 0; i < cubeNum; i++) {
		Cube* cube = cubeRef[i];

		cubeModelVBOs[i] = linearAlloc(sizeof(cube_sides_lut));
		memcpy(cubeModelVBOs[i], cube->vertices, sizeof(cube_sides_lut));
		free((void*)cube->vertices);
		cube->vboIdx = i;
	}
}
void Cube_DeinitVBOs() {
	for (u16 i = 0; i < cubeNum; i++) {
		linearFree(cubeModelVBOs[i]);
	}
}

Cube* Cube_Init(const CubeRaw* in, s16 texwidth, s16 texheight) {
	if (!in) {
		Crash("Passed unbaked CubeRaw is NULL!");
		return NULL;
	}

	Cube* cube = (Cube*)malloc(sizeof(Cube));

	cube->vertices = (WorldVertex*)malloc(sizeof(cube_sides_lut));

	memcpy(cube->vertices, cube_sides_lut, sizeof(cube_sides_lut));

#define VERTEX_SCALE 0.941f

	float from[3], to[3];
	for (u8 i = 0; i < 3; ++i) {
		from[i] = in->from[i] * VERTEX_SCALE;
		to[i]	= in->to[i] * VERTEX_SCALE;
	}

	const s16 length = to[0] / VERTEX_SCALE;
	const s16 width	 = to[2] / VERTEX_SCALE;
	const s16 height = to[1] / VERTEX_SCALE;

	s16 faceUV[4];
	for (u8 face = 0; face < 6; ++face) {
		const u8 lutStartIndex = face * 6;

		// Calculate Texture offsets using Coordinates(1 2DP=1 3DP)
		switch (face) {
			case Direction_West:
				faceUV[0] = in->texOffset[0] + width;
				faceUV[1] = in->texOffset[1] + width * 2;
				faceUV[2] = in->texOffset[0];
				faceUV[3] = in->texOffset[1] + width;
				break;
			case Direction_East:
				faceUV[0] = in->texOffset[0] + width + length;
				faceUV[1] = in->texOffset[1] + width + height;
				faceUV[2] = in->texOffset[0] + width * 2 + length;
				faceUV[3] = in->texOffset[1] + width;
				break;
			case Direction_Bottom:
				faceUV[0] = in->texOffset[0] + width;
				faceUV[1] = in->texOffset[1] + width;
				faceUV[2] = in->texOffset[0] + width + length;
				faceUV[3] = in->texOffset[1];
				break;
			case Direction_Top:
				faceUV[0] = in->texOffset[0] + width + length;
				faceUV[1] = in->texOffset[1];
				faceUV[2] = in->texOffset[0] + width * 2 + length;
				faceUV[3] = in->texOffset[1] + width;
				break;
			case Direction_North:
				faceUV[0] = in->texOffset[0] + width * 2 + length * 2;
				faceUV[1] = in->texOffset[1] + width + height;
				faceUV[2] = in->texOffset[0] + width * 2 + length;
				faceUV[3] = in->texOffset[1] + width;
				break;
			case Direction_South:
				faceUV[0] = in->texOffset[0] + width;
				faceUV[1] = in->texOffset[1] + width + height;
				faceUV[2] = in->texOffset[0] + width + length;
				faceUV[3] = in->texOffset[1] + width;
				break;
		}

		// Apply transformations for each vertex in the LUT
		for (int i = 0; i < 6; ++i) {
			u8 idx				= lutStartIndex + i;
			WorldVertex* vertex = &cube->vertices[idx];

			vertex->pos[0] = -(in->mirrored ? -from[0] : from[0] + (cube_sides_lut[idx].pos[0] ? in->mirrored ? -to[0] : to[0] : 0));
			vertex->pos[1] = -(from[1] + (cube_sides_lut[idx].pos[1] ? to[1] : 0));
			vertex->pos[2] = -(from[2] + (cube_sides_lut[idx].pos[2] ? to[2] : 0));

#define toTexCrd(x, tw) (s16)(((float)(x) / (float)(tw)) * (float)((1 << 15) - 1))

			s16 u = faceUV[cube_sides_lut[idx].uv[0] * 2], v = faceUV[cube_sides_lut[idx].uv[1] * 2 + 1];
			u = v;
			v = u;

			vertex->uv[0] = toTexCrd(faceUV[cube_sides_lut[idx].uv[0] * 2], texwidth);
			vertex->uv[1] = toTexCrd(faceUV[cube_sides_lut[idx].uv[1] * 2 + 1], texheight);
		}
	}
	C3D_Mtx matrix;
	Mtx_Identity(&matrix);
	Mtx_RotateX(&matrix, in->rotation[0], true);
	Mtx_RotateY(&matrix, in->rotation[1], true);
	Mtx_RotateZ(&matrix, in->rotation[2], true);

	Mtx_Copy(&cube->localMatrix, &matrix);

	cubeRef[cubeNum] = cube;
	cubeNum++;
	return cube;
}

void Cube_Deinit(Cube* cube) {
	if (cube == NULL)
		return;

	linearFree(cube->vertices);
	linearFree(cube);
}

void Cube_Draw(Cube* cube, int shaderUniform, C3D_Mtx* matrix) {
	C3D_Mtx outMatrix;
	Mtx_Identity(&outMatrix);
	Mtx_Multiply(&outMatrix, matrix, &cube->localMatrix);

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, shaderUniform, &outMatrix);

	WorldVertex* vbo = cubeModelVBOs[cube->vboIdx];

	if (vbo == NULL)
		Crash("Cube Num %d has NULL VBO!", cube->vboIdx);

	GSPGPU_FlushDataCache(vbo, sizeof(cube_sides_lut));

	C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, vbo, sizeof(WorldVertex), 4, 0x3210);

	C3D_DrawArrays(GPU_TRIANGLES, 0, CUBE_VERTICE_NUM);
}

void Cube_Reset(Cube* c) {
	Mtx_Identity(&c->localMatrix);
}
void Cube_SetPos(Cube* cube, float3 pos) {
	Mtx_Translate(&cube->localMatrix, pos.x, pos.y, pos.z, true);
}
void Cube_SetRot(Cube* c, float3 rot) {
	Mtx_RotateX(&c->localMatrix, rot.x, true);
	Mtx_RotateY(&c->localMatrix, rot.y, true);
	Mtx_RotateZ(&c->localMatrix, rot.z, true);
}