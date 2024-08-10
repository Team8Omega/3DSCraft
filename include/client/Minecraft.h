#pragma once

#include "client/gui/GuiScreen.h"
#include "world/World.h"
#include "world/chunk/ChunkWorker.h"
#include "world/level/storage/SaveManager.h"

void gInit(const char* username, bool isNew3ds, bool isDemo);
void gRun();
void gDisplayPauseMenu();
void gSetScreen(u8 idx);

void gStop();

void gLoadWorld(char* path, char* name, WorldGenType worldType, bool newWorld);
void gReleaseWorld();

extern bool gSkipWorldRender;