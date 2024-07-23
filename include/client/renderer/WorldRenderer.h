#pragma once

#include "client/player/Player.h"
#include "world/WorkQueue.h"
#include "world/World.h"

#include "client/Camera.h"

void WorldRenderer_Init(Player* player_, World* world_, WorkQueue* workqueue_, int projectionUniform_);
void WorldRenderer_Deinit();

void WorldRenderer_Render(float iod);
