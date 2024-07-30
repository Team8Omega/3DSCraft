#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "core/Direction.h"

typedef u8 Item;

enum
{
	Item_Totem,
	Items_Count
};

void Item_Init();
void Item_Deinit();

void* Item_GetTextureMap();

void Item_GetTexture(Item items, Direction direction, u8 metadata, s16* out_uv);

extern const char* ItemNames[Items_Count];
