#pragma once

#include "world/World.h"
#include "world/chunk/ChunkWorker.h"
#include "world/level/storage/SaveManager.h"

void Game_LoadWorld(char* path, char* name, WorldGenType worldType, bool newWorld);
void Game_ReleaseWorld();