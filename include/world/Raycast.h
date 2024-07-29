#pragma once

#include "world/World.h"

#include "util/math/VecMath.h"

typedef struct {
	int x, y, z;
	float distSqr;
	Direction direction;
} Raycast_Result;

bool Raycast_Cast(float3 inpos, float3 raydir, Raycast_Result* out);
