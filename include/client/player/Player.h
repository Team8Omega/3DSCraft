#pragma once

#include "world/World.h"
#include <stdbool.h>

#include "client/gui/Inventory.h"
#include "client/model/CubeModel.h"
#include "client/player/DamageDef.h"

#include "sounds/Sound.h"
#include "util/math/VecMath.h"
#include "world/Raycast.h"
#include "world/level/item/ItemStack.h"

#define PLAYER_EYEHEIGHT (1.65f)
#define PLAYER_HEIGHT (1.8f)
#define PLAYER_COLLISIONBOX_SIZE (0.65f)
#define PLAYER_HALFEYEDIFF (0.07f)

#define PLAYER_PLACE_REPLACE_TIMEOUT (0.2f)

typedef struct {
	float3 position;
	float pitch, yaw;
	float bobbing, fovAdd, crouchAdd;
	bool grounded, jumped, sprinting, flying, crouching;

	float3 view;

	bool autoJumpEnabled;

	float3 velocity;
	float simStepAccum;

	float breakPlaceTimeout;
	int hungertimer;

	int hp;
	int armour;
	int oxygen;
	int hunger;
	float experience;
	float experienceLevel;
	int difficulty;
	float rndy;

	float spawnx;
	float spawny;
	float spawnz;
	int spawnset;

	int gamemode;
	bool cheats;

	int quickSelectBarSlots;
	int quickSelectBarSlot;
	int inventorySite;
	ItemStack quickSelectBar[INVENTORY_QUICKSELECT_MAXSLOTS];

	Raycast_Result viewRayCast;
	bool blockInSight, blockInActionRange;
	ItemStack inventory[64];

	CubeModel* model;

	Damage dmg;
} Player;

extern Player gPlayer;

void Player_Init();

void Player_InitModel();

void Player_Draw(int projectionUniform, C3D_Mtx* matrix);

void Player_Update(Sound* sound);

void Player_Move(float dt, float3 accl);

void Player_PlaceBlock(Sound* sound);
void Player_BreakBlock();

void Player_Jump(float3 accl);

void Player_Deinit();