#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <citro3d.h>
#include <sino/sino.h>

#include <3ds.h>

#include "Minecraft.h"

#include "client/Crash.h"
#include "client/gui/DebugUI.h"
#include "client/gui/Gui.h"
#include "client/gui/ScreenManager.h"
#include "client/gui/screens/PauseScreen.h"
#include "client/gui/screens/TitleScreen.h"
#include "client/model/Cube.h"
#include "client/player/Damage.h"
#include "client/player/Player.h"
#include "client/player/PlayerController.h"
#include "client/renderer/PolyGen.h"
#include "client/renderer/Renderer.h"
#include "sounds/Sound.h"
#include "util/Paths.h"
#include "util/StringUtils.h"
#include "world/level/levelgen/SmeaGen.h"
#include "world/level/levelgen/SuperFlatGen.h"
#include "world/level/storage/Region.h"

#include "client/language/LanguageManager.h"

#include "Globals.h"

InputData gInput, gInputOld;
bool gIsNew3ds;

static ChunkWorker chunkWorker;
static SuperFlatGen flatGen;
static SmeaGen smeaGen;
static Sound BackgroundSound;
static Sound PlayerSound;
static PlayerController playerCtrl;
static SaveManager savemgr;

static u64 lastTime, currentTime;
static float dt = 0.f, timeAccum = 0.f, fpsClock = 0.f;
static int frameCounter = 0, fps = 0;

void initCheck() {
	// Check for block asset
	if (access(PATH_PACKS PACK_VANILLA "/" PATH_PACK_TEXTURES "/"
									   "block/stone.png",
			   F_OK))
		Crash(
			"Please provide assets, check\n \'github.com/Team8Omega/3DSCraft-ResourcePacker\'\nfor infos.\nYou have to validly provide "
			"your own assets of the game you purchased, as we do not support piracy.\n\nFILENOTFOUND: "
			"block/stone.png");

	// Check for valid license
	if (access("romfs:/licenses.txt", F_OK))
		Crash(
			"This build is shipped without license information for third parties, and is therefore not legit.\nPlease build with "
			"\'licenses.txt\' in "
			"romfs, and retry.");

	APT_CheckNew3DS(&gIsNew3ds);
}

int main() {
	gfxInitDefault();
	romfsInit();

	initCheck();

	osSetSpeedupEnable(true);  // Enable N3DS 804MHz operation, where available
	gfxSet3D(true);

	Region_InitPools();

	SaveManager_InitFileSystem();

	ChunkWorker_Init(&chunkWorker);
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_PolyGen, (WorkerFuncObj){ &PolyGen_GeneratePolygons, NULL, true });
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_BaseGen, (WorkerFuncObj){ &SuperFlatGen_Generate, &flatGen, true });
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_BaseGen, (WorkerFuncObj){ &SmeaGen_Generate, &smeaGen, true });

	sino_init();

	Player_Init();
	PlayerController_Init(&playerCtrl);

	World_Init(&chunkWorker.queue);

	SuperFlatGen_Init(&flatGen);
	SmeaGen_Init(&smeaGen);

	Renderer_Init(&chunkWorker.queue);

	DebugUI_Init();

	test();

	SaveManager_Init(&savemgr);
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_Load, (WorkerFuncObj){ &SaveManager_LoadChunk, &savemgr, true });
	ChunkWorker_AddHandler(&chunkWorker, WorkerItemType_Save, (WorkerFuncObj){ &SaveManager_SaveChunk, &savemgr, true });

	BackgroundSound.background = true;
	BackgroundSound.path	   = String_ParsePackName(PACK_VANILLA, PATH_PACK_SOUNDS, "music/1.opus");
	Sound_PlayOpus(&BackgroundSound);

	Cube_InitVBOs();  // needs to be called AFTER all cubes have been built.

	lastTime = svcGetSystemTick();

	while (aptMainLoop()) {
		DebugUI_Text("%d FPS  CPU: %5.2f%% GPU: %5.2f%% Buf: %5.2f%% Lin: %d", fps, C3D_GetProcessingTime() * 6.f,
					 C3D_GetDrawingTime() * 6.f, C3D_GetCmdBufUsage() * 100.f, linearSpaceFree());

		if (!currentScreen) {
			DebugUI_Text("X: %f, Y: %f, Z: %f", f3_unpack(gPlayer.position));
			DebugUI_Text("HP: %i", gPlayer.hp);
			// DebugUI_Text("velocity: %f rndy: %f",gPlayer.velocity.y,gPlayer.rndy);
			// DebugUI_Text("Damage Time: %i Cause: %c",dmg->time,dmg->cause);
			// DebugUI_Text("Spawn X: %f Y: %f Z: %f",gPlayer.spawnx,gPlayer.spawny,gPlayer.spawnz);
			DebugUI_Text("Hunger: %i Hungertimer: %i", gPlayer.hunger, gPlayer.hungertimer);
			// DebugUI_Text("Gamemode: %i", gPlayer.gamemode);
			// DebugUI_Text("quickbar %i",gPlayer.quickSelectBarSlot);}
		}

		Renderer_Render();

		currentTime = svcGetSystemTick();
		dt			= ((float)(currentTime / (float)CPU_TICKS_PER_MSEC) - (float)(lastTime / (float)CPU_TICKS_PER_MSEC)) / 1000.f;
		lastTime	= currentTime;
		timeAccum += dt;

		frameCounter++;
		fpsClock += dt;
		if (fpsClock >= 1.f) {
			fps			 = frameCounter;
			frameCounter = 0;
			fpsClock	 = 0.f;
		}

		circlePosition circlePos, cstickPos;
		touchPosition touchPos;

		hidScanInput();
		hidCircleRead(&circlePos);
		hidCstickRead(&cstickPos);
		hidTouchRead(&touchPos);

		gInputOld = gInput;
		gInput	  = (InputData){ hidKeysHeld(), hidKeysDown(), hidKeysUp(),	 circlePos.dx, circlePos.dy,
								 touchPos.px,	touchPos.py,   cstickPos.dx, cstickPos.dy };

		if (currentScreen) {
			ScreenManager_Update();
		} else {
			if (gInput.keysdown & KEY_START) {
				ScreenManager_SetScreen(&sPauseScreen);
			}
			while (timeAccum >= 1.f / 20.f) {
				World_Tick();

				timeAccum -= 1.f / 20.f;
			}
			PlayerController_Update(&playerCtrl, &PlayerSound, dt);
		}

		if (gWorld.active)
			World_UpdateChunkCache(WorldToChunkCoord(FastFloor(gPlayer.position.x)), WorldToChunkCoord(FastFloor(gPlayer.position.z)));
	}

	if (gWorld.active)
		Game_ReleaseWorld();

	SaveManager_Deinit(&savemgr);

	Region_DeinitPools();

	Sound_Quit(0);
	threadJoin(BackgroundSound.threaid, 50000);
	threadFree(BackgroundSound.threaid);
	if (BackgroundSound.opusFile) {
		op_free(BackgroundSound.opusFile);
	}
	Sound_Deinit(0);

	Sound_Quit(1);
	threadJoin(PlayerSound.threaid, 50000);
	threadFree(PlayerSound.threaid);
	if (PlayerSound.opusFile) {
		op_free(PlayerSound.opusFile);
	}
	Sound_Deinit(1);

	ndspExit();
	sino_exit();

	DebugUI_Deinit();

	ChunkWorker_Deinit(&chunkWorker);

	Renderer_Deinit();

	Cube_DeinitVBOs();

	romfsExit();

	gfxExit();
	return 0;
}

void Game_ReleaseWorld() {
	for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
		for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
			World_UnloadChunk(gWorld.chunkCache[i][j]);
		}
	}
	ChunkWorker_Finish(&chunkWorker);
	World_Reset();

	SaveManager_Unload(&savemgr);
}

void Game_LoadWorld(char* path, char* name, WorldGenType worldType, bool newWorld) {
	// Crash("path: %s\nname: %s\nworldtype: %d\nnewWorld: %s", path, name, worldType, newWorld ? "true" : "false");
	strcpy(gWorld.name, name);
	gWorld.genSettings.type = worldType;

	SaveManager_Load(&savemgr, path);

	ChunkWorker_SetHandlerActive(&chunkWorker, WorkerItemType_BaseGen, &flatGen, gWorld.genSettings.type == WorldGen_SuperFlat);
	ChunkWorker_SetHandlerActive(&chunkWorker, WorkerItemType_BaseGen, &smeaGen, gWorld.genSettings.type == WorldGen_Default);

	gWorld.cacheTranslationX = WorldToChunkCoord(FastFloor(gPlayer.position.x));
	gWorld.cacheTranslationZ = WorldToChunkCoord(FastFloor(gPlayer.position.z));
	for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
		for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
			gWorld.chunkCache[i][j] =
				World_LoadChunk(i - CHUNKCACHE_SIZE / 2 + gWorld.cacheTranslationX, j - CHUNKCACHE_SIZE / 2 + gWorld.cacheTranslationZ);
		}
	}

	for (int i = 0; i < 3; i++) {
		while (chunkWorker.working || chunkWorker.queue.queue.length > 0) {
			svcSleepThread(50000000);  // 1 Tick
		}
		World_Tick();
	}

	if (newWorld) {
		int highestblock = 0;
		for (int x = -1; x < 1; x++) {
			for (int z = -1; z < 1; z++) {
				int height = World_GetHeight(x, z);
				if (height > highestblock)
					highestblock = height;
			}
		}
		gPlayer.hunger	   = 20;
		gPlayer.hp		   = 20;
		gPlayer.position.y = (float)highestblock + 0.2f;
	}
	ScreenManager_SetScreen(NULL);
	gWorld.active = true;
	lastTime	  = svcGetSystemTick();	 // fix timing
}