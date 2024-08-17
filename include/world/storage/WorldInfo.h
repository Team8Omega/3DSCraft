#pragma once

#include "world/GameRules.h"

#define WORLD_NAME_SIZE 128
typedef struct {
	u64 seed;
	char name[WORLD_NAME_SIZE];
	char path[WORLD_NAME_SIZE];
	int spawnx, spawny, spawnz;
	u64 gameTime;
	u64 dayTime;
	GameRules gamerules;
} WorldInfo;