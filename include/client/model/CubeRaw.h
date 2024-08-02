#pragma once

#define CUBE_MAX_NUM 16

typedef struct {
	const s16 pos[3], size[3];
	const s16 texOffset[2];
} CubeRaw;

typedef struct {
	const s16 dimensions[2];
	CubeRaw cubes[CUBE_MAX_NUM];
} CubeModelDef;