#pragma once

#include "world/WorkQueue.h"
#include "world/World.h"

typedef struct {
	bool placeholder;
} SuperFlatGen;

void SuperFlatGen_Init(SuperFlatGen* gen);

void SuperFlatGen_Generate(WorkerItem item, void* this);
