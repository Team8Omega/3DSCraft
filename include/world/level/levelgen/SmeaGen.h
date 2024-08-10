#pragma once

#include "world/World.h"

typedef struct {
	bool placeholder;
} SmeaGen;

void SmeaGen_Init(SmeaGen* gen);
void SmeaGen_Generate(WorkerItem item, void* this);
