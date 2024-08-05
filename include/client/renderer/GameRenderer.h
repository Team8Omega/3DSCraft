#pragma once

#include "client/player/Player.h"
#include "world/WorkQueue.h"
#include "world/World.h"
#include <Globals.h>

void GameRenderer_Init(WorkQueue* queue);
void GameRenderer_Deinit();

void GameRenderer_Tick();
void GameRenderer_Render();
