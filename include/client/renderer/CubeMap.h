#pragma once

#include <citro3d.h>

#include "util/math/VecMath.h"

void CubeMap_Init(int projUniform_);
void CubeMap_Set(const char* basename, float3 rotation);
void CubeMap_Deinit();

void CubeMap_Render();

void CubeMap_Tick(C3D_Mtx* projection, float3 rotationOffset);
