#pragma once

#include "client/player/Player.h"
#include "world/WorkQueue.h"
#include "world/World.h"
#include <Globals.h>


void Renderer_Init(World* world_, Player* player_, WorkQueue* queue);
void Renderer_Deinit();

void Renderer_Render();
