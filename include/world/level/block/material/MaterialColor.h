#pragma once

#include <3ds/types.h>

typedef enum
{
	COLOR_AIR = 0,
	COLOR_GRASS,
	COLOR_SAND,
	COLOR_WOOL,
	COLOR_TNT,
	COLOR_ICE,
	COLOR_IRON,
	COLOR_FOLIAGE,
	COLOR_SNOW,
	COLOR_CLAY,
	COLOR_DIRT,
	COLOR_STONE,
	COLOR_WATER,
	COLOR_WOOD,
	COLOR_COUNT
} MaterialColor;

extern const u16 COLORS[COLOR_COUNT];