#pragma once

#include "client/player/Player.h"
#include "world/WorkQueue.h"
#include "world/World.h"

#include "client/model/VertexFmt.h"

void PolyGen_Init(World* world_, Player* player_);
void PolyGen_Deinit();

void PolyGen_Harvest();
void PolyGen_GeneratePolygons(WorkQueue* queue, WorkerItem item, void* this);
