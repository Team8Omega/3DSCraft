#include "client/player/PlayerController.h"

#include "client/Camera.h"
#include "commands/CommandLine.h"
#include "util/math/NumberUtils.h"

#include "client/gui/DebugUI.h"
#include "util/Paths.h"

#include "Globals.h"
#include "sounds/Sound.h"

#include <ini/ini.h>
#include <unistd.h>

#ifdef _3DS
#include <3ds.h>
#define PLATFORM_BUTTONS 23
const char* platform_key_names[PLATFORM_BUTTONS] = { "Not Set",	   "A",			 "B",			"X",		"Y",		 "L",
													 "R",		   "Start",		 "Select",		"DUp",		"DDown",	 "DLeft",
													 "DRight",	   "CircUp",	 "CircDown",	"CircLeft", "CircRight", "CStickUp",
													 "CStickDown", "CStickLeft", "CStickRight", "ZL",		"ZR" };
enum
{
	K3DS_NONE = 0,
	K3DS_A,
	K3DS_B,
	K3DS_X,
	K3DS_Y,
	K3DS_L,
	K3DS_R,
	K3DS_START,
	K3DS_SELECT,
	K3DS_DUP,
	K3DS_DDOWN,
	K3DS_DLEFT,
	K3DS_DRIGHT,
	K3DS_CPAD_UP,
	K3DS_CPAD_DOWN,
	K3DS_CPAD_LEFT,
	K3DS_CPAD_RIGHT,
	K3DS_CSTICK_UP,
	K3DS_CSTICK_DOWN,
	K3DS_CSTICK_LEFT,
	K3DS_CSTICK_RIGHT,
	K3DS_ZL,
	K3DS_ZR
};

const PlayerControlScheme platform_default_scheme = { .forward			= K3DS_CPAD_UP,
													  .backward			= K3DS_CPAD_DOWN,
													  .strafeLeft		= K3DS_CPAD_LEFT,
													  .strafeRight		= K3DS_CPAD_RIGHT,
													  .lookLeft			= K3DS_Y,
													  .lookRight		= K3DS_A,
													  .lookUp			= K3DS_X,
													  .lookDown			= K3DS_B,
													  .placeBlock		= K3DS_L,
													  .breakBlock		= K3DS_R,
													  .jump				= K3DS_DUP,
													  .switchBlockLeft	= K3DS_NONE,
													  .switchBlockRight = K3DS_NONE,
													  .openCmd			= K3DS_SELECT,
													  .crouch			= K3DS_DLEFT,
													  .camMode			= K3DS_DDOWN };

const PlayerControlScheme n3ds_default_scheme = { .forward			= K3DS_CPAD_UP,
												  .backward			= K3DS_CPAD_DOWN,
												  .strafeLeft		= K3DS_CPAD_LEFT,
												  .strafeRight		= K3DS_CPAD_RIGHT,
												  .lookLeft			= K3DS_CSTICK_LEFT,
												  .lookRight		= K3DS_CSTICK_RIGHT,
												  .lookUp			= K3DS_CSTICK_UP,
												  .lookDown			= K3DS_CSTICK_DOWN,
												  .placeBlock		= K3DS_L,
												  .breakBlock		= K3DS_R,
												  .jump				= K3DS_A,
												  .switchBlockLeft	= K3DS_ZL,
												  .switchBlockRight = K3DS_ZR,
												  .openCmd			= K3DS_SELECT,
												  .crouch			= K3DS_B,
												  .camMode			= K3DS_DDOWN };

static void convertPlatformInput(InputData* input, float ctrls[], bool keysdown[], bool keysup[]) {
#define reg_bin_key(i, k)                                                                                                                  \
	ctrls[(i)]	  = (float)((input->keysdown & (k)) || (input->keysheld & (k)));                                                           \
	keysdown[(i)] = input->keysdown & (k);                                                                                                 \
	keysup[(i)]	  = input->keysup & (k)

	ctrls[0]	= 0.f;
	keysdown[0] = 0;
	keysup[0]	= 0;

	reg_bin_key(K3DS_A, KEY_A);
	reg_bin_key(K3DS_B, KEY_B);
	reg_bin_key(K3DS_X, KEY_X);
	reg_bin_key(K3DS_Y, KEY_Y);

	reg_bin_key(K3DS_L, KEY_L);
	reg_bin_key(K3DS_R, KEY_R);

	reg_bin_key(K3DS_START, KEY_START);
	reg_bin_key(K3DS_SELECT, KEY_SELECT);

	reg_bin_key(K3DS_DUP, KEY_DUP);
	reg_bin_key(K3DS_DDOWN, KEY_DDOWN);
	reg_bin_key(K3DS_DLEFT, KEY_DLEFT);
	reg_bin_key(K3DS_DRIGHT, KEY_DRIGHT);

	reg_bin_key(K3DS_ZL, KEY_ZL);
	reg_bin_key(K3DS_ZR, KEY_ZR);

	float circX = (float)input->circlePadX / 0x9c;
	float circY = (float)input->circlePadY / 0x9c;

#define reg_cpad_key(i, c, k)                                                                                                              \
	ctrls[(i)]	  = ABS(c) * (float)((input->keysdown & k) || (input->keysheld & k));                                                      \
	keysdown[(i)] = input->keysdown & (k);                                                                                                 \
	keysup[(i)]	  = input->keysup & (k)

	reg_cpad_key(K3DS_CPAD_UP, circY, KEY_CPAD_UP);
	reg_cpad_key(K3DS_CPAD_DOWN, circY, KEY_CPAD_DOWN);
	reg_cpad_key(K3DS_CPAD_LEFT, circX, KEY_CPAD_LEFT);
	reg_cpad_key(K3DS_CPAD_RIGHT, circX, KEY_CPAD_RIGHT);

	float cstickX = (float)input->cStickX / 0x9c;
	float cstickY = (float)input->cStickY / 0x9c;

	reg_cpad_key(K3DS_CSTICK_UP, cstickY, KEY_CSTICK_UP);
	reg_cpad_key(K3DS_CSTICK_DOWN, cstickY, KEY_CSTICK_DOWN);
	reg_cpad_key(K3DS_CSTICK_LEFT, cstickX, KEY_CSTICK_LEFT);
	reg_cpad_key(K3DS_CSTICK_RIGHT, cstickX, KEY_CSTICK_RIGHT);

#undef reg_bin_key
}
#else
#error "Input conversion functions not implemented for this platform"
#endif

typedef struct {
	float keys[PLATFORM_BUTTONS];
	bool keysup[PLATFORM_BUTTONS];
	bool keysdown[PLATFORM_BUTTONS];
} PlatformAgnosticInput;

static inline float IsKeyDown(KeyCombo combo, PlatformAgnosticInput* input) {
	return input->keys[combo];
}
static inline bool WasKeyReleased(KeyCombo combo, PlatformAgnosticInput* input) {
	return input->keysup[combo];
}
static inline bool WasKeyPressed(KeyCombo combo, PlatformAgnosticInput* input) {
	return input->keysdown[combo];
}

void PlayerController_Init(PlayerController* ctrl) {
	ctrl->breakPlaceTimeout = 0.f;

	if (gIsNew3ds) {
		ctrl->controlScheme = n3ds_default_scheme;
	} else {
		ctrl->controlScheme = platform_default_scheme;
	}

	ctrl->openedCmd = false;

	bool elementMissing = false;

	const char path[] = PATH_ROOT "options.ini";
	if (access(path, F_OK) != -1) {
		ini_t* cfg = ini_load(path);

		char buffer[64];

#define loadKey(variable)                                                                                                                  \
	if (ini_sget(cfg, "controls", #variable, "%s", buffer)) {                                                                              \
		for (int i = 0; i < PLATFORM_BUTTONS; i++) {                                                                                       \
			if (!strcmp(platform_key_names[i], buffer)) {                                                                                  \
				ctrl->controlScheme.variable = i;                                                                                          \
				break;                                                                                                                     \
			}                                                                                                                              \
		}                                                                                                                                  \
	} else                                                                                                                                 \
		elementMissing = true;

		loadKey(forward);
		loadKey(backward);
		loadKey(strafeLeft);
		loadKey(strafeRight);
		loadKey(lookLeft);
		loadKey(lookRight);
		loadKey(lookUp);
		loadKey(lookDown);
		loadKey(placeBlock);
		loadKey(breakBlock);
		loadKey(jump);
		loadKey(switchBlockLeft);
		loadKey(switchBlockRight);
		loadKey(openCmd);
		loadKey(crouch);
		loadKey(camMode);
#undef loadKey

		if (!ini_sget(cfg, "controls", "auto_jumping", "%d", &gPlayer->autoJumpEnabled))
			//	elementMissing = true;

			ini_free(cfg);

	} else
		elementMissing = true;

	if (elementMissing) {
		FILE* f = fopen(path, "w");

		fprintf(f, "[controls]\n");
		fprintf(f, "; The allowed key values are: \n; ");
		int j = 0;
		for (int i = 0; i < PLATFORM_BUTTONS - 1; i++) {
			fprintf(f, "%s, ", platform_key_names[i]);
			if (++j == 5) {
				j = 0;
				fprintf(f, "\n ; ");
			}
		}
		fprintf(f, "%s\n\n", platform_key_names[PLATFORM_BUTTONS - 1]);

#define writeKey(key) fprintf(f, #key "=%s\n", platform_key_names[ctrl->controlScheme.key]);

		writeKey(forward);
		writeKey(backward);
		writeKey(strafeLeft);
		writeKey(strafeRight);
		writeKey(lookLeft);
		writeKey(lookRight);
		writeKey(lookUp);
		writeKey(lookDown);
		writeKey(placeBlock);
		writeKey(breakBlock);
		writeKey(jump);
		writeKey(switchBlockLeft);
		writeKey(switchBlockRight);
		writeKey(openCmd);
		writeKey(crouch);
		writeKey(camMode);

#undef writeKey

		fprintf(f, "; 0 = disabled, 1 = enabled\nautojump=%d\n", true);

		fclose(f);
	}

	ctrl->flyTimer = -1.f;
}

void PlayerController_Tick(PlayerController* ctrl, Sound* sound, float dt) {
	if (!gPlayer || !gWorld)
		return;

	PlatformAgnosticInput agnosticInput;
	convertPlatformInput(&gInput, agnosticInput.keys, agnosticInput.keysdown, agnosticInput.keysup);

	float jump	 = IsKeyDown(ctrl->controlScheme.jump, &agnosticInput);
	float crouch = IsKeyDown(ctrl->controlScheme.crouch, &agnosticInput);

	float forward	  = IsKeyDown(ctrl->controlScheme.forward, &agnosticInput);
	float backward	  = IsKeyDown(ctrl->controlScheme.backward, &agnosticInput);
	float strafeLeft  = IsKeyDown(ctrl->controlScheme.strafeLeft, &agnosticInput);
	float strafeRight = IsKeyDown(ctrl->controlScheme.strafeRight, &agnosticInput);

	float3 forwardVec = f3_new(-sinf(gPlayer->yaw), 0.f, -cosf(gPlayer->yaw));
	float3 rightVec	  = f3_crs(forwardVec, f3_new(0, 1, 0));

	float3 movement = f3_new(0, 0, 0);
	movement		= f3_add(movement, f3_scl(forwardVec, forward));
	movement		= f3_sub(movement, f3_scl(forwardVec, backward));
	movement		= f3_add(movement, f3_scl(rightVec, strafeRight));
	movement		= f3_sub(movement, f3_scl(rightVec, strafeLeft));
	if (gPlayer->flying) {
		movement = f3_add(movement, f3_new(0.f, jump, 0.f));
		movement = f3_sub(movement, f3_new(0.f, crouch, 0.f));
	}
	if (f3_magSqr(movement) > 0.f) {
		float speed = 4.3f * f3_mag(f3_new(-strafeLeft + strafeRight, -crouch + jump, -forward + backward));
		gPlayer->bobbing += speed * 1.5f * dt;
		movement = f3_scl(f3_nrm(movement), speed);
	}

	float lookLeft	= IsKeyDown(ctrl->controlScheme.lookLeft, &agnosticInput);
	float lookRight = IsKeyDown(ctrl->controlScheme.lookRight, &agnosticInput);
	float lookUp	= IsKeyDown(ctrl->controlScheme.lookUp, &agnosticInput);
	float lookDown	= IsKeyDown(ctrl->controlScheme.lookDown, &agnosticInput);

	gPlayer->yaw += (lookLeft + -lookRight) * 160.f * DEG_TO_RAD * dt;
	gPlayer->pitch += (-lookDown + lookUp) * 160.f * DEG_TO_RAD * dt;
	gPlayer->pitch = CLAMP(gPlayer->pitch, -DEG_TO_RAD * 89.9f, DEG_TO_RAD * 89.9f);

	float placeBlock = IsKeyDown(ctrl->controlScheme.placeBlock, &agnosticInput);
	float breakBlock = IsKeyDown(ctrl->controlScheme.breakBlock, &agnosticInput);
	if (placeBlock > 0.f)
		Player_PlaceBlock(sound);
	if (breakBlock > 0.f)
		Player_BreakBlock();

	if (jump > 0.f)
		Player_Jump(movement);

	bool releasedJump = WasKeyReleased(ctrl->controlScheme.jump, &agnosticInput);
	if (ctrl->flyTimer >= 0.f) {
		if (jump > 0.f)
			gPlayer->flying ^= true;
		ctrl->flyTimer += dt;
		if (ctrl->flyTimer > 0.25f)
			ctrl->flyTimer = -1.f;
	} else if (releasedJump) {
		ctrl->flyTimer = 0.f;
	}

	bool releasedCrouch = WasKeyReleased(ctrl->controlScheme.crouch, &agnosticInput);
	gPlayer->crouching ^= !gPlayer->flying && releasedCrouch;

	bool switchBlockLeft  = WasKeyPressed(ctrl->controlScheme.switchBlockLeft, &agnosticInput);
	bool switchBlockRight = WasKeyPressed(ctrl->controlScheme.switchBlockRight, &agnosticInput);
	if (switchBlockLeft && --gPlayer->quickSelectBarSlot == -1)
		gPlayer->quickSelectBarSlot = INVENTORY_QUICKSELECT_MAXSLOTS - 1;
	if (switchBlockRight && ++gPlayer->quickSelectBarSlot == INVENTORY_QUICKSELECT_MAXSLOTS)
		gPlayer->quickSelectBarSlot = 0;

	if (ctrl->openedCmd) {
		dt				= 0.f;
		ctrl->openedCmd = false;
	}

	float cmdLine = WasKeyPressed(ctrl->controlScheme.openCmd, &agnosticInput);
	if (cmdLine) {
		CommandLine_Activate();
		ctrl->openedCmd = true;
	}

	bool camMode = WasKeyPressed(ctrl->controlScheme.camMode, &agnosticInput);
	if (camMode) {
		gCamera.mode = (gCamera.mode + 1) % CameraMode_Count;
	}

	Player_Move(dt, movement);
	Player_Tick(sound);
}
