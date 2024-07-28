#pragma once

#define CUBE_MAX_NUM 16

typedef struct {
	const s16 from[3], to[3];
	const s16 texOffset[2];
	const float rotation[3];
	bool mirrored;
} CubeRaw;

typedef struct {
	const s16 dimensions[2];
	CubeRaw cubes[CUBE_MAX_NUM];
} CubeModelDef;