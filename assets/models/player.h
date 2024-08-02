#include "client/model/CubeRaw.h"

typedef enum
{
	MODELPART_PLAYER_HEAD,
	MODELPART_PLAYER_HEADWEAR,
	MODELPART_PLAYER_LEGR,
	MODELPART_PLAYER_BODY,
	MODELPART_PLAYER_ARMR,
	MODELPART_PLAYER_LEGRWEAR,
	MODELPART_PLAYER_BODYWEAR,
	MODELPART_PLAYER_ARMRWEAR,
	MODELPART_PLAYER_LEGLWEAR,
	MODELPART_PLAYER_LEGL,
	MODELPART_PLAYER_ARML,
	MODELPART_PLAYER_ARMLWEAR,
	MODELPART_PLAYER_COUNT
} MODELPART_PLAYER;

#define rotator M_PI

static CubeModelDef modeldef_player = { { 64, 64 },
										{
											{ // head
											  { -4, -12, -4 },
											  { 8, 8, 8 },
											  { 0, 0 } },
											{ // body
											  { -4, -4, -2 },
											  { 8, 12, 4 },
											  { 16, 16 } },
											{ // arm r
											  { -4, -4, -2 },
											  { -4, 12, 4 },
											  { 40, 16 } },
											{ // arm l
											  { 4, -4, -2 },
											  { 4, 12, 4 },
											  { 32, 48 } },
											{ // leg r
											  { 0, 8, -2 },
											  { 4, 12, 4 },
											  { 0, 16 } },
											{ // leg l
											  { -4, 8, -2 },
											  { 4, 12, 4 },
											  { 16, 48 } },
										}

};
/*
{ // head 1
// ToDo: Cube is flipped, please unflip
{ -3.765f, 0.f, -3.765f },
{ 3.765f, 7.529f, 3.765f },
{ 64, 64 },
{ // right, left, top, bottom, back, front
// x1, y1, x2, y2
{ 0, 8, 8, 16 },
{ 16, 8, 24, 16 },
{ 8, 0, 16, 8 },
{ 16, 0, 24, 8 },
{ 8, 8, 16, 16 },
{ 24, 8, 32, 16 } },
{ 0, 22.589, -offsetZ },
{ 0, 0, 0 } },
{ // head 2
// ToDo: Cube is flipped, please unflip
// spacing is currently +1, in og its 0.25 i believe
{ -4.765f, 0.0f, -4.765f },
{ 4.765f, 8.529f, 4.765f },
{ 64, 64 },
{
// right, left, top, bottom, back, front
// x1, y1, x2, y2
{ 32, 8, 40, 16 },  // r
{ 48, 8, 56, 16 },  // l

{ 40, 0, 48, 8 },	 // t
{ 48, 0, 56, 8 },	 // b

{ 56, 8, 64, 16 },  // b
{ 40, 8, 48, 16 },  // f
},
{ 0, 22.589, -offsetZ },
{ 0, 0, 0 } },

{ // leg r
// ToDo: Leg is rotated 180° to the wrong side on x
{ -1.882f, 0.f, -1.882f },
{ 1.882f, -11.294f, 1.882f },
{ 64, 64 },
{ // right, left, top, bottom, back, front
// x1, y1, x2, y2
{ 0, 20, 4, 32 },
{ 8, 20, 12, 32 },
{ 4, 16, 8, 20 },
{ 8, 16, 12, 20 },
{ 12, 20, 16, 32 },
{ 4, 20, 8, 32 } },
{ -1.882, 11.294, -offsetZ },
{ 0, 0, 0 } },
{ // body
{ -3.765f, 0.f, 1.882f },
{ 3.765f, -11.295f, -1.882f },
{ 64, 64 },
{ // right, left, top, bottom, back, front
// x1, y1, x2, y2
{ 16, 20, 20, 32 },
{ 28, 20, 32, 32 },
{ 20, 16, 28, 20 },
{ 28, 16, 36, 20 },
{ 32, 20, 40, 32 },
{ 20, 20, 28, 32 } },
{ 0, 22.589f, -offsetZ },
{ 0, 0, 0 } },
{ // arm r
{ 1.882f, 1.882f, 1.882f },
{ -1.882f, -9.413f, -1.882f },
{ 64, 64 },
{ // right, left, top, bottom, back, front
// x1, y1, x2, y2
{ 40, 20, 44, 32 },
{ 48, 20, 52, 32 },
{ 44, 16, 48, 20 },
{ 48, 16, 52, 20 },
{ 52, 20, 56, 32 },
{ 44, 20, 48, 32 } },
{ 5.647, 20.707, -offsetZ },
{ 0, 0, 0 } },

// layer 2
{ // leg r
// ToDo: Leg is rotated 180° to the wrong side on x
{ -2.882f, 0.f, -2.882f },
{ 2.882f, -12.294f, 2.882f },
{ 64, 64 },
{ // right, left, top, bottom, back, front
// x1, y1, x2, y2
{ 0, 36, 4, 48 },
{ 8, 36, 12, 48 },
{ 4, 32, 8, 36 },
{ 8, 32, 12, 36 },
{ 12, 36, 16, 48 },
{ 4, 36, 8, 48 } },
{ -1.882, 11.294, -offsetZ },
{ 0, 0, 0 } },
{ // body
{ -4.765f, 0.f, 2.882f },
{ 4.765f, -12.295f, -2.882f },
{ 64, 64 },
{ // right, left, top, bottom, back, front
// x1, y1, x2, y2
{ 16, 36, 20, 48 },
{ 28, 36, 32, 48 },
{ 20, 32, 28, 36 },
{ 28, 32, 36, 36 },
{ 32, 36, 40, 48 },
{ 20, 36, 28, 48 } },
{ 0, 22.589f, -offsetZ },
{ 0, 0, 0 } },
{ // arm r
{ 2.882f, 2.882f, 2.882f },
{ -2.882f, -10.413f, -2.882f },
{ 64, 64 },
{ // right, left, top, bottom, back, front
// x1, y1, x2, y2
{ 40, 36, 44, 48 },
{ 48, 36, 52, 48 },
{ 44, 32, 48, 36 },
{ 48, 32, 48, 32 },
{ 52, 36, 56, 48 },
{ 44, 36, 48, 48 } },
{ 5.647, 20.707, -offsetZ },
{ 0, 0, 0 } },

// layer 2
{ // leg l
// ToDo: Leg is rotated 180° to the wrong side on x
{ -2.882f, 0.f, -2.882f },
{ 2.882f, -12.294f, 2.882f },
{ 64, 64 },
{ // right, left, top, bottom, back, front
// x1, y1, x2, y2
{ 0, 52, 4, 64 },
{ 8, 52, 12, 64 },
{ 4, 48, 8, 52 },
{ 8, 48, 12, 52 },
{ 15, 52, 16, 64 },
{ 4, 52, 8, 64 } },
{ 1.882, 11.294, -offsetZ },
{ 0, 0, 0 } },

// layer 1
{ // leg l
// ToDo: Leg is rotated 180° to the wrong side on x
{ -1.882f, 0.f, -1.882f },
{ 1.882f, -11.294f, 1.882f },
{ 64, 64 },
{ // right, left, top, bottom, back, front
// x1, y1, x2, y2
{ 16, 52, 20, 64 },
{ 24, 52, 28, 64 },
{ 24, 52, 20, 48 },
{ 28, 52, 24, 48 },
{ 28, 52, 32, 64 },
{ 20, 52, 24, 64 } },
{ 1.882, 11.294, -offsetZ },
{ 0, 0, 0 } },

// layer 1
{ // arm l
{ -1.882f, 1.882f, -1.882f },
{ 1.882f, -9.413f, 1.882f },
{ 64, 64 },
{
// right, left, top, bottom, back, front
// x1, y1, x2, y2
{ 40, 20, 44, 32 },
{ 48, 20, 52, 32 },
{ 44, 16, 48, 20 },
{ 48, 16, 52, 20 },
{ 52, 20, 56, 32 },
{ 44, 20, 48, 32 },
},
{ -5.647, 20.707, -offsetZ },
{ 0, 0, 0 } },
// layer 2
{ // arm l
{ -2.882f, 2.882f, -2.882f },
{ 2.882f, -10.413f, 2.882f },
{ 64, 64 },
{
// right, left, top, bottom, back, front
// x1, y1, x2, y2
{ 48, 52, 52, 64 },
{ 56, 52, 60, 64 },
{ 52, 48, 56, 52 },
{ 56, 48, 60, 52 },
{ 60, 52, 64, 64 },
{ 52, 52, 56, 64 },
},
{ -5.647, 20.707, -offsetZ },
{ 0, 0, 0 } },
};*/