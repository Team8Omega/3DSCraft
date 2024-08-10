#pragma once

#include "client/gui/Screen.h"
#include "world/World.h"

#include <vec/vec.h>

typedef struct {
	u32 lastPlayed;
	char name[256];
	char path[256];
} WorldInfo;

extern WorldGenType worldGenType;
extern Gamemode gamemode;
extern Difficulty difficulty;
extern const char* worldGenTypesStr[];
extern const char* gamemodeTypesStr[];
extern const char* difficultyTypesStr[];

typedef struct {
	vec_t(WorldInfo) list;
} WorldList;
extern WorldList worlds;

extern Screen sCreateWorldScreen;

void CreateWorldScreen(u16 selectedIdx, u16 worldNo);