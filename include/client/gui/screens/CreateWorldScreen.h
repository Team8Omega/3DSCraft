#pragma once

#include "client/gui/Screen.h"
#include "world/World.h"
#include "world/storage/WorldSummary.h"

#include <vec/vec.h>

extern WorldGenType worldGenType;
extern Gamemode gamemode;
extern Difficulty difficulty;
extern const char* worldGenTypesStr[];
extern const char* gamemodeTypesStr[];
extern const char* difficultyTypesStr[];

typedef struct {
	vec_t(WorldSummary) list;
} WorldList;
extern WorldList worlds;

extern Screen sCreateWorldScreen;

void CreateWorldScreen(u16 selectedIdx, u16 worldNo);