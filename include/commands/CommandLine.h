#pragma once

#include "client/player/Player.h"
#include "world/World.h"

void CommandLine_Activate(World* world, Player* player);
void CommandLine_Execute(World* world, Player* player, const char* text);
