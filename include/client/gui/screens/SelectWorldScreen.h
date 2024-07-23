#pragma once

#include "client/player/Player.h"
#include <stdbool.h>
#include <stdint.h>

#include "world/World.h"

typedef struct {
		char worldName[WORLD_NAME_SIZE];
		GeneratorSettings settings;
} WorldSelect_Result;

void SelectWorldScreen_Init();
void SelectWorldScreen_Deinit();

void SelectWorldScreen_Render();
bool SelectWorldScreen_Update(char* out_worldpath, char* out_name, WorldGenType* worldType, bool* newWorld);

void SelectWorldScreen_ScanWorlds();
