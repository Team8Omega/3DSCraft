#pragma once

#include "world/WorkQueue.h"
#include "world/World.h"

#include "client/Camera.h"

void WorldRenderer_Init(int projectionUniform_);
void WorldRenderer_Deinit();

void WorldRenderer_Tick();
void WorldRenderer_Render();
