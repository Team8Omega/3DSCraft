#pragma once

#include <citro3d.h>

#include "world/World.h"

void Clouds_Init();
void Clouds_Deinit();

void Clouds_Tick(float tx, float ty, float tz);
void Clouds_Render(int projUniform, C3D_Mtx* projectionview);
