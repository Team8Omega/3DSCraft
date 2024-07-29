#pragma once

#include "client/player/Player.h"
#include "world/WorkQueue.h"
#include "world/World.h"
#include <Globals.h>

void Renderer_Init(WorkQueue* queue);
void Renderer_Deinit();

void Renderer_Render();
