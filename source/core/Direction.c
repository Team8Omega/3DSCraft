#include "core/Direction.h"

// West, East, Bottom, Top, North, South

s8 DirectionToOffset[7][3] = {
	{ -1, 0, 0 }, { 1, 0, 0 }, { 0, -1, 0 }, { 0, 1, 0 }, { 0, 0, -1 }, { 0, 0, 1 }, { 0, 0, 0 },
};

Direction DirectionOpposite[7] = {
	Direction_East, Direction_West, Direction_Top, Direction_Bottom, Direction_South, Direction_North, Direction_None,
};

Axis DirectionToAxis[7] = {
	Axis_X, Axis_X, Axis_Y, Axis_Y, Axis_Z, Axis_Z, Axis_X,
};

const char* DirectionNames[7] = { "west", "east", "down", "up", "north", "south", "" };

#include <string.h>

Direction DirectionByName(const char* name) {
	for (u8 i = 0; i < 6; ++i) {
		if (strcmp(name, DirectionNames[i]) == 0)
			return (Direction)i;
	}
	return Direction_None;
}