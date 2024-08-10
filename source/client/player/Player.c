#include "client/player/Player.h"

#include "client/Camera.h"
#include "client/Crash.h"
#include "client/gui/DebugUI.h"
#include "client/player/Damage.h"
#include "client/renderer/texture/TextureMap.h"
#include "sounds/Sound.h"
#include "util/Paths.h"
#include "util/StringUtils.h"
#include "world/phys/Collision.h"

#include "assets/models.h"
#include "entity/Entity.h"

Player* gPlayer;

static C3D_Tex textureSkin;
static CubeModel* cubeTMP;
static Entity playerEntity;	 // TODO: rework entity

void Player_Init() {
	Texture_Load(&textureSkin, "entity/player/wide/steve.png");

	cubeTMP		 = createModel(&modeldef_player);
	playerEntity = (Entity){ .modelType = cubeTMP, .texture = &textureSkin };
}

void Player_Draw() {
	playerEntity.position = gPlayer->position;
	++playerEntity.position.y;
	playerEntity.yaw = gPlayer->yaw - DEG_TO_RAD * 180;
	Entity_Draw(&playerEntity);
}

void Player_Deinit() {
	CubeModel_Deinit(cubeTMP);
	C3D_TexDelete(&textureSkin);
}

void Player_Unload() {
	free(gPlayer);
}

void Player_Load() {
	gPlayer = (Player*)malloc(sizeof(Player));

	gPlayer->active = true;

	gPlayer->position = f3_new(0.f, 0.f, 0.f);
	gPlayer->spawnPos = f3_new(0.f, 0.f, 0.f);

	gPlayer->bobbing = 0.f;
	gPlayer->pitch	 = 0.f;
	gPlayer->yaw	 = 0.f;

	gPlayer->grounded	= false;
	gPlayer->sprinting	= false;
	gPlayer->gamemode	= 0;
	gPlayer->difficulty = 1;

	gPlayer->fovAdd		 = 0.f;
	gPlayer->crouchAdd	 = 0.f;
	gPlayer->hungertimer = 0;

	gPlayer->view = f3_new(0, 0, -1);

	gPlayer->crouching = false;
	gPlayer->flying	   = false;

	gPlayer->blockInSight		= false;
	gPlayer->blockInActionRange = false;

	gPlayer->velocity	  = f3_new(0, 0, 0);
	gPlayer->simStepAccum = 0.f;

	gPlayer->breakPlaceTimeout = 0.f;

	gPlayer->quickSelectBarSlot = 0;
	gPlayer->inventorySite		= 1;

	memset(gPlayer->inventory, 0, sizeof(ItemStack) * 64);
	for (u8 i = 1; i < BLOCK_COUNT; ++i) {
		// if(i != BLOCK_WOOL)
		gPlayer->inventory[i - 1] = (ItemStack){ i, 0, 1 };
	}

	for (int i = 0; i < INVENTORY_QUICKSELECT_MAXSLOTS; i++)
		gPlayer->quickSelectBar[i] = (ItemStack){ BLOCK_AIR, 0, 0 };

	gPlayer->autoJumpEnabled = false;

	// hardcoded values for now
	// experience is a value between 0 and 0.99*
	gPlayer->experience		 = 0.1;
	gPlayer->experienceLevel = 5000000;
}

void Player_Tick(Sound* sound) {
	gPlayer->view = f3_new(-sinf(gPlayer->yaw) * cosf(gPlayer->pitch), sinf(gPlayer->pitch), -cosf(gPlayer->yaw) * cosf(gPlayer->pitch));
	gPlayer->blockInSight		= Raycast_Cast(f3_new(gPlayer->position.x, gPlayer->position.y + PLAYER_EYEHEIGHT, gPlayer->position.z),
										   gPlayer->view, &gPlayer->viewRayCast);
	gPlayer->blockInActionRange = gPlayer->blockInSight && gPlayer->viewRayCast.distSqr < 3.5f * 3.5f * 3.5f;
	// if (gPlayer->gamemode!=1){
	// Fall damage
	if (gPlayer->velocity.y <= -12) {
		// gPlayer->rndy;
		gPlayer->rndy = round(gPlayer->velocity.y);
		if (World_GetBlock(gPlayer->position.x, gPlayer->position.y - 1, gPlayer->position.z) != BLOCK_AIR) {
			gPlayer->hp	  = gPlayer->hp + gPlayer->rndy;
			gPlayer->rndy = 0;
		}
	}
	// Fire damage
	// if (World_GetBlock(f3_unpack(gPlayer->position)) ==
	//	Block_Lava /*||World_GetBlock(gPlayer->world,f3_unpack(gPlayer->position)) == Block_Fire*/) {
	//	DebugUI_Log("ur burning lol");
	//	gPlayer->dmg = (Damage){ .cause = DAMAGECAUSE_FIRE, .time = 10 };
	//	OvertimeDamage();
	//}
	// Hunger
	// if (gPlayer->difficulty!=0){
	//  1000000000 having this here as reference on how long 1 second is
	// svcSleepThread(10000000);
	gPlayer->hungertimer = gPlayer->hungertimer + 1;
	if (gPlayer->hungertimer == 400 && gPlayer->hunger != 0) {
		gPlayer->hunger		 = gPlayer->hunger - 1;
		gPlayer->hungertimer = 0;
	}
	if (gPlayer->hunger == 0) {
		svcSleepThread(10000000);
		if (gPlayer->hungertimer == 400) {
			gPlayer->hp			 = gPlayer->hp - 1;
			gPlayer->hungertimer = 0;
		}
	}
	//}
	// Respawning stuff
	if (gPlayer->hp <= 0 /*&&gPlayer->totem==false*/) {
		if (gPlayer->difficulty != 4) {
			if (gPlayer->dmg.cause == DAMAGECAUSE_NONE) {
				DebugUI_Log("Player died");
			} else {
				DebugUI_Log("Died by %s", gPlayer->dmg.cause);
			}
			int spawnY = FastFloor(gPlayer->spawnPos.y);
			if (World_GetBlock(gPlayer->spawnPos.x, spawnY, gPlayer->spawnPos.z) != BLOCK_AIR) {
				while (World_GetBlock(gPlayer->spawnPos.x, spawnY, gPlayer->spawnPos.z) != BLOCK_AIR)
					spawnY++;
			}

			gPlayer->position.y = spawnY;
			gPlayer->position.x = gPlayer->spawnPos.x;
			gPlayer->position.z = gPlayer->spawnPos.z;

			gPlayer->hp		   = 20;
			gPlayer->hunger	   = 20;
			gPlayer->dmg.cause = DAMAGECAUSE_NONE;
		} else {
			DebugUI_Log("lol ur world is gone");
			/*char buffer[512];
			sprintf(buffer, PATH_SAVES "%s", worlds.data[selectedWorld].path);
			delete_folder(buffer);*/
		}
	}
	//}
}

bool Player_CanMove(float3 new) {
	for (int x = -1; x < 2; x++) {
		for (int y = 0; y < 3; y++) {
			for (int z = -1; z < 2; z++) {
				int pX = FastFloor(new.x) + x;
				int pY = FastFloor(new.y) + y;
				int pZ = FastFloor(new.z) + z;
				if (World_GetBlock(pX, pY, pZ) != BLOCK_AIR /* && World_GetBlock(pX, pY, pZ) != Block_Lava &&
					World_GetBlock(pX, pY, pZ) != Block_Water */) {
					if (AABB_Overlap(new.x - PLAYER_COLLISIONBOX_SIZE / 2.f, new.y, new.z - PLAYER_COLLISIONBOX_SIZE / 2.f,
									 PLAYER_COLLISIONBOX_SIZE, PLAYER_HEIGHT, PLAYER_COLLISIONBOX_SIZE, pX, pY, pZ, 1.f, 1.f, 1.f)) {
						return false;
					}
				}
			}
		}
	}
	return true;
}

void Player_Jump(float3 accl) {
	if (gPlayer->grounded && !gPlayer->flying) {
		gPlayer->velocity.x = accl.x * 1.1f;
		gPlayer->velocity.z = accl.z * 1.1f;
		gPlayer->velocity.y = 6.7f;
		gPlayer->jumped		= true;
		gPlayer->crouching	= false;
	}
}
#include "client/gui/DebugUI.h"
const float MaxWalkVelocity		= 4.3f;
const float MaxFallVelocity		= -50.f;
const float GravityPlusFriction = 10.f;
void Player_Move(float dt, float3 accl) {
	if (!gPlayer || !gWorld)
		return;

	gPlayer->breakPlaceTimeout -= dt;
	gPlayer->simStepAccum += dt;
	const float SimStep = 1.f / 60.f;
	while (gPlayer->simStepAccum >= SimStep) {
		gPlayer->velocity.y -= GravityPlusFriction * SimStep * 2.f;
		if (gPlayer->velocity.y < MaxFallVelocity)
			gPlayer->velocity.y = MaxFallVelocity;

		if (gPlayer->flying)
			gPlayer->velocity.y = 0.f;

		float speedFactor = 1.f;
		if (!gPlayer->grounded && !gPlayer->flying) {
			if (gPlayer->jumped)
				speedFactor = 0.2f;
			else
				speedFactor = 0.6f;
		} else if (gPlayer->flying)
			speedFactor = 2.f;
		else if (gPlayer->crouching)
			speedFactor = 0.5f;
		float3 newPos	= f3_add(gPlayer->position, f3_add(f3_scl(gPlayer->velocity, SimStep), f3_scl(accl, SimStep * speedFactor)));
		float3 finalPos = gPlayer->position;

		bool wallCollision = false, wasGrounded = gPlayer->grounded;

		gPlayer->grounded = false;
		for (int j = 0; j < 3; j++) {
			int i			= (int[]){ 0, 2, 1 }[j];
			bool collision	= false;
			float3 axisStep = /*f3_new(i == 0 ? newPos.x : gPlayer->position.x, i == 1 ? newPos.y : gPlayer->position.y,
									 i == 2 ? newPos.z : gPlayer->position.z)*/
				finalPos;
			axisStep.v[i] = newPos.v[i];
			Box playerBox = Box_Create(axisStep.x - PLAYER_COLLISIONBOX_SIZE / 2.f, axisStep.y, axisStep.z - PLAYER_COLLISIONBOX_SIZE / 2.f,
									   PLAYER_COLLISIONBOX_SIZE, PLAYER_HEIGHT, PLAYER_COLLISIONBOX_SIZE);

			for (int x = -1; x < 2; x++) {
				for (int y = 0; y < 3; y++) {
					for (int z = -1; z < 2; z++) {
						int pX = FastFloor(axisStep.x) + x;
						int pY = FastFloor(axisStep.y) + y;
						int pZ = FastFloor(axisStep.z) + z;
						if (World_GetBlock(pX, pY, pZ) != BLOCK_AIR) {
							Box blockBox = Box_Create(pX, pY, pZ, 1, 1, 1);

							float3 normal = f3_new(0.f, 0.f, 0.f);
							float depth	  = 0.f;
							int face	  = 0;

							bool intersects = Collision_BoxIntersect(blockBox, playerBox, 0, &normal, &depth, &face);
							collision |= intersects;
						}
					}
				}
			}
			if (!collision)
				finalPos.v[i] = newPos.v[i];
			else if (i == 1) {
				if (gPlayer->velocity.y < 0.f || accl.y < 0.f)
					gPlayer->grounded = true;
				gPlayer->jumped		= false;
				gPlayer->velocity.x = 0.f;
				gPlayer->velocity.y = 0.f;
				gPlayer->velocity.z = 0.f;
			} else {
				wallCollision = true;
				if (i == 0)
					gPlayer->velocity.x = 0.f;
				else
					gPlayer->velocity.z = 0.f;
			}
		}

		float3 movDiff = f3_sub(finalPos, gPlayer->position);

		if (gPlayer->grounded && gPlayer->flying)
			gPlayer->flying = false;

		if (wallCollision && gPlayer->autoJumpEnabled) {
			float3 nrmDiff = f3_nrm(f3_sub(newPos, gPlayer->position));
			BlockId block =
				World_GetBlock(FastFloor(finalPos.x + nrmDiff.x), FastFloor(finalPos.y + nrmDiff.y) + 2, FastFloor(finalPos.z + nrmDiff.z));
			BlockId landingBlock =
				World_GetBlock(FastFloor(finalPos.x + nrmDiff.x), FastFloor(finalPos.y + nrmDiff.y) + 1, FastFloor(finalPos.z + nrmDiff.z));
			if (block == BLOCK_AIR && landingBlock != BLOCK_AIR)
				Player_Jump(accl);
		}

		if (gPlayer->crouching && gPlayer->crouchAdd > -0.3f)
			gPlayer->crouchAdd -= SimStep * 2.f;
		if (!gPlayer->crouching && gPlayer->crouchAdd < 0.0f)
			gPlayer->crouchAdd += SimStep * 2.f;

		if (gPlayer->crouching && !gPlayer->grounded && wasGrounded && finalPos.y < gPlayer->position.y && movDiff.x != 0.f &&
			movDiff.z != 0.f) {
			finalPos			= gPlayer->position;
			gPlayer->grounded	= true;
			gPlayer->velocity.y = 0.f;
		}

		gPlayer->position = finalPos;
		gPlayer->velocity = f3_new(gPlayer->velocity.x * 0.95f, gPlayer->velocity.y, gPlayer->velocity.z * 0.95f);
		if (ABS(gPlayer->velocity.x) < 0.1f)
			gPlayer->velocity.x = 0.f;
		if (ABS(gPlayer->velocity.z) < 0.1f)
			gPlayer->velocity.z = 0.f;

		gPlayer->simStepAccum -= SimStep;
	}
}

void Player_PlaceBlock(Sound* sound) {
	if (gPlayer->blockInActionRange && gPlayer->breakPlaceTimeout < 0.f) {
		const s8* offset = DirectionToOffset[gPlayer->viewRayCast.direction];
		int x = gPlayer->viewRayCast.x + offset[0], y = gPlayer->viewRayCast.y + offset[1], z = gPlayer->viewRayCast.z + offset[2];

		if (AABB_Overlap(gPlayer->position.x - PLAYER_COLLISIONBOX_SIZE / 2.f, gPlayer->position.y,
						 gPlayer->position.z - PLAYER_COLLISIONBOX_SIZE / 2.f, PLAYER_COLLISIONBOX_SIZE, PLAYER_HEIGHT,
						 PLAYER_COLLISIONBOX_SIZE, x, y, z, 1.f, 1.f, 1.f))
			return;

		ItemStack slotNow = gPlayer->quickSelectBar[gPlayer->quickSelectBarSlot];
		World_SetBlockAndMeta(x, y, z, slotNow.block, slotNow.meta);

		/*sound->background = false;
		sound->path		  = String_ParsePackName(PACK_VANILLA, PATH_PACK_SOUNDS, "entity/player/hit.opus");
		// DebugUI_Log("File path for player sound %s", sound->path);
		Sound_PlayOpus(sound);*/ // ffs we are writing a *nice* sound class wrapper to handle this, stop hardcoding.
	}
	if (gPlayer->breakPlaceTimeout < 0.f)
		gPlayer->breakPlaceTimeout = PLAYER_PLACE_REPLACE_TIMEOUT;
}

void Player_BreakBlock() {
	if (gPlayer->blockInActionRange && gPlayer->breakPlaceTimeout < 0.f) {
		World_SetBlock(gPlayer->viewRayCast.x, gPlayer->viewRayCast.y, gPlayer->viewRayCast.z, BLOCK_AIR);
	}
	if (gPlayer->breakPlaceTimeout < 0.f)
		gPlayer->breakPlaceTimeout = PLAYER_PLACE_REPLACE_TIMEOUT;
}
