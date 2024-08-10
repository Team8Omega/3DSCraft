#pragma once

#include "client/player/Player.h"
#include "world/WorkQueue.h"
#include "world/World.h"

void PolyGen_Init();
void PolyGen_Deinit();

void PolyGen_Harvest();
void PolyGen_GeneratePolygons(WorkerItem item, void* this);
