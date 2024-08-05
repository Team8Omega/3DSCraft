#include "client/Minecraft.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <citro3d.h>
#include <sino/sino.h>

#include <3ds.h>

#include "client/Minecraft.h"

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
#include "client/renderer/GameRenderer.h"
#include "client/renderer/PolyGen.h"
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

bool gSkipWorldRender;

static ChunkWorker sChunkWorker;
static SuperFlatGen sFlatGen;
static SmeaGen smeaGen;
static Sound sBackgroundSound;
static Sound sPlayerSound;
static PlayerController sPlayerCtrl;
static SaveManager sSavemgr;

static u64 sLastTime, sCurrentTime;
static float sTimeAccum = 0.f, sFpsClock = 0.f, sTickClock = 0.f;
static int sFrameCounter = 0, sFps = 0, sTickCounter = 0, sTps = 0;

static bool sRunning;
static char* sUsername;
static bool sIsNew3ds;
static bool sIsDemo;

void Game_Init(const char* name, bool isNew, bool demo) {
	sUsername = (char*)malloc(strlen(name) + 1);
	strcpy(sUsername, name);
	sIsNew3ds = isNew;
	sIsDemo	  = demo;

	// temporary
	gIsNew3ds = isNew;
}

static void init() {
	gfxInitDefault();
	osSetSpeedupEnable(true);  // Enable N3DS 804MHz operation, where available
	gfxSet3D(true);

	Region_InitPools();

	SaveManager_InitFileSystem();

	ChunkWorker_Init(&sChunkWorker);
	ChunkWorker_AddHandler(&sChunkWorker, WorkerItemType_PolyGen, (WorkerFuncObj){ &PolyGen_GeneratePolygons, NULL, true });
	ChunkWorker_AddHandler(&sChunkWorker, WorkerItemType_BaseGen, (WorkerFuncObj){ &SuperFlatGen_Generate, &sFlatGen, true });
	ChunkWorker_AddHandler(&sChunkWorker, WorkerItemType_BaseGen, (WorkerFuncObj){ &SmeaGen_Generate, &smeaGen, true });

	sino_init();

	Player_Init();
	PlayerController_Init(&sPlayerCtrl);

	World_Init(&sChunkWorker.queue);

	SuperFlatGen_Init(&sFlatGen);
	SmeaGen_Init(&smeaGen);

	GameRenderer_Init(&sChunkWorker.queue);

	DebugUI_Init();

	test();

	SaveManager_Init(&sSavemgr);
	ChunkWorker_AddHandler(&sChunkWorker, WorkerItemType_Load, (WorkerFuncObj){ &SaveManager_LoadChunk, &sSavemgr, true });
	ChunkWorker_AddHandler(&sChunkWorker, WorkerItemType_Save, (WorkerFuncObj){ &SaveManager_SaveChunk, &sSavemgr, true });

	ndspInit();
	sBackgroundSound.background = true;
	sBackgroundSound.path		= String_ParsePackName(PACK_VANILLA, PATH_PACK_SOUNDS, "music/1.opus");
	Sound_PlayOpus(&sBackgroundSound);

	Cube_InitVBOs();  // needs to be called AFTER all cubes have been built.

	sLastTime = svcGetSystemTick();
}

static void deinit() {
	if (gWorld.active) {
		Game_ReleaseWorld();
	}

	SaveManager_Deinit(&sSavemgr);

	Region_DeinitPools();

	Sound_Quit(0);
	threadJoin(sBackgroundSound.threaid, 50000);
	threadFree(sBackgroundSound.threaid);
	if (sBackgroundSound.opusFile) {
		op_free(sBackgroundSound.opusFile);
	}
	Sound_Deinit(0);

	Sound_Quit(1);
	threadJoin(sPlayerSound.threaid, 50000);
	threadFree(sPlayerSound.threaid);
	if (sPlayerSound.opusFile) {
		op_free(sPlayerSound.opusFile);
	}
	Sound_Deinit(1);

	ndspExit();
	sino_exit();

	DebugUI_Deinit();

	ChunkWorker_Deinit(&sChunkWorker);

	GameRenderer_Deinit();

	Cube_DeinitVBOs();

	romfsExit();

	gfxExit();
}

static void runGameLoop() {
	// Display debugging information
	DebugUI_Text("%d FPS %d TPS  CPU %5.2f%% GPU %5.2f%% Buf %5.2f%% Lin %d", sFps, sTps, C3D_GetProcessingTime() * 6.f,
				 C3D_GetDrawingTime() * 6.f, C3D_GetCmdBufUsage() * 100.f, linearSpaceFree());

	if (gWorld.active && !currentScreen) {
		DebugUI_Text("X: %f, Y: %f, Z: %f", f3_unpack(gPlayer.position));
		DebugUI_Text("HP: %i", gPlayer.hp);
		// DebugUI_Text("velocity: %f rndy: %f",gPlayer.velocity.y,gPlayer.rndy);
		// DebugUI_Text("Damage Time: %i Cause: %c",dmg->time,dmg->cause);
		// DebugUI_Text("Spawn X: %f Y: %f Z: %f",gPlayer.spawnx,gPlayer.spawny,gPlayer.spawnz);
		DebugUI_Text("Hunger: %i Hungertimer: %i", gPlayer.hunger, gPlayer.hungertimer);
		// DebugUI_Text("Gamemode: %i", gPlayer.gamemode);
		// DebugUI_Text("quickbar %i",gPlayer.quickSelectBarSlot);}
	}

	sCurrentTime	= svcGetSystemTick();
	float frameTime = ((float)(sCurrentTime / (float)CPU_TICKS_PER_MSEC) - (float)(sLastTime / (float)CPU_TICKS_PER_MSEC)) / 1000.f;
	sLastTime		= sCurrentTime;
	sTimeAccum += frameTime;

	sFrameCounter++;
	sFpsClock += frameTime;
	if (sFpsClock >= 1.f) {
		sFps		  = sFrameCounter;
		sFrameCounter = 0;
		sFpsClock	  = 0.f;
	}

	sTickClock += frameTime;
	if (sTickClock >= 1.f) {
		sTps		 = sTickCounter;
		sTickCounter = 0;
		sTickClock	 = 0.f;
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

	// OG Game has Simulations capped to 20 fps and render is X fps
	const float tickRate = 1.f / 20.f;
	// const float tickDt	 = frameTime * (60 / 20);  // once variable fps is in, replace with 60. keep 20.
	while (sTimeAccum >= tickRate) {
		GameRenderer_Tick();

		sTimeAccum -= tickRate;
		sTickCounter++;
	}
	PlayerController_Update(&sPlayerCtrl, &sPlayerSound, frameTime);

	GameRenderer_Render();
}

void Game_Run() {
	sRunning = true;

	init();

	while (sRunning && aptMainLoop()) {
		runGameLoop();
	}

	deinit();
}

void Game_Stop() {
	sRunning = false;
}

void Game_ReleaseWorld() {
	for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
		for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
			World_UnloadChunk(gWorld.chunkCache[i][j]);
		}
	}
	ChunkWorker_Finish(&sChunkWorker);
	World_Reset();

	SaveManager_Unload(&sSavemgr);
}

void Game_LoadWorld(char* path, char* name, WorldGenType worldType, bool newWorld) {
	// Crash("path: %s\nname: %s\nworlsDtype: %d\nnewWorld: %s", path, name, worldType, newWorld ? "true" : "false");
	strcpy(gWorld.name, name);
	gWorld.genSettings.type = worldType;

	SaveManager_Load(&sSavemgr, path);

	ChunkWorker_SetHandlerActive(&sChunkWorker, WorkerItemType_BaseGen, &sFlatGen, gWorld.genSettings.type == WorldGen_SuperFlat);
	ChunkWorker_SetHandlerActive(&sChunkWorker, WorkerItemType_BaseGen, &smeaGen, gWorld.genSettings.type == WorldGen_Default);

	gWorld.cacheTranslationX = WorldToChunkCoord(FastFloor(gPlayer.position.x));
	gWorld.cacheTranslationZ = WorldToChunkCoord(FastFloor(gPlayer.position.z));
	for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
		for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
			gWorld.chunkCache[i][j] =
				World_LoadChunk(i - CHUNKCACHE_SIZE / 2 + gWorld.cacheTranslationX, j - CHUNKCACHE_SIZE / 2 + gWorld.cacheTranslationZ);
		}
	}

	for (int i = 0; i < 3; i++) {
		while (sChunkWorker.working || sChunkWorker.queue.queue.length > 0) {
			svcSleepThread(50000000);  // 1 Tick
		}
		World_UpdateChunkGen();
	}

	if (newWorld) {
		int highestblock = 0;
		for (int x = -1; x < 1; ++x) {
			for (int z = -1; z < 1; ++z) {
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
	sLastTime	  = svcGetSystemTick();	 // fix timing
}