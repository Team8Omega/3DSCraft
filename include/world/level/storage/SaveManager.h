#pragma once

#include <stdio.h>

#include <vec/vec.h>

#include "client/player/Player.h"

#include "world/level/storage/Region.h"

typedef struct {
	vec_t(Region*) regions;
} SaveManager;

void SaveManager_InitFileSystem();

void SaveManager_Init(SaveManager* mgr);
void SaveManager_Deinit(SaveManager* mgr);

void SaveManager_Load(SaveManager* mgr, const char* path);
void SaveManager_Unload(SaveManager* mgr);

void SaveManager_LoadChunk(WorkQueue* queue, WorkerItem item, void* this);
void SaveManager_SaveChunk(WorkQueue* queue, WorkerItem item, void* this);
