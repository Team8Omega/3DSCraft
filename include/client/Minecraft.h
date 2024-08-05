#pragma once

#include "world/World.h"
#include "world/chunk/ChunkWorker.h"
#include "world/level/storage/SaveManager.h"

void Game_Init(const char* username, bool isNew3ds, bool isDemo);
void Game_Run();

void Game_Stop();

void Game_LoadWorld(char* path, char* name, WorldGenType worldType, bool newWorld);
void Game_ReleaseWorld();

extern bool gSkipWorldRender;