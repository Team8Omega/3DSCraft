#pragma once

typedef u8 Direction;
enum {
	Direction_West,
	Direction_East,
	Direction_Bottom,
	Direction_Top,
	Direction_North,
	Direction_South,
	Direction_None
};

typedef u8 Axis;
enum {
	Axis_X,
	Axis_Y,
	Axis_Z
};

extern s8 DirectionToOffset[7][3];
extern Direction DirectionOpposite[7];
extern Axis DirectionToAxis[7];
