#pragma once

#include <Globals.h>
#include "client/player/Player.h"
#include "world/WorkQueue.h"
#include "world/World.h"

void Renderer_Init(World* world_, Player* player_, WorkQueue* queue, GameState* gamestate_);
void Renderer_Deinit();

void Renderer_Render();
