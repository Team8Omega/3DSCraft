#include "client/Game.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <citro3d.h>
#include <sino/sino.h>

#include <3ds.h>

#include "client/Crash.h"
#include "client/gui/DebugUI.h"
#include "client/gui/Gui.h"
#include "client/gui/IngameGui.h"
#include "client/gui/Screen.h"
#include "client/gui/screens/PauseScreen.h"
#include "client/gui/screens/TitleScreen.h"
#include "client/model/Cube.h"
#include "client/player/Damage.h"
#include "client/player/Player.h"
#include "client/player/PlayerController.h"
#include "client/renderer/GameRenderer.h"
#include "client/renderer/PolyGen.h"
#include "client/renderer/SpriteBatch.h"
#include "client/renderer/WorldRenderer.h"
#include "client/renderer/block/model/BlockModel.h"
#include "resources/locale/LanguageManager.h"
#include "sounds/Sound.h"
#include "util/Paths.h"
#include "util/StringUtils.h"
#include "world/GameRules.h"
#include "world/level/levelgen/SmeaGen.h"
#include "world/level/levelgen/SuperFlatGen.h"
#include "world/storage/Region.h"

#include "Globals.h"

#define FPS 60

InputData gInput, gInputOld;
bool gIsNew3ds;

bool gSkipWorldRender;

static ChunkWorker sChunkWorker;
static SuperFlatGen sFlatGen;
static SmeaGen smeaGen;
// static Sound sBackgroundSound;
static Sound sPlayerSound;
static PlayerController sPlayerCtrl;
static SaveManager sSavemgr;

static u64 sLastTime, sCurrentTime;
static float sTimeAccum = 0.f, sFpsClock = 0.f;
float sDt				= 0.f;
static u8 sFrameCounter = 0, sFps = 0, sTickCounter = 0, sTps = 0;

static aptHookCookie sAptHook;

static bool sRunning;
static bool sHasInited;
static bool sHasExited;
static char* sUsername;
static bool sIsNew3ds;
static bool sIsDemo;
static float s3dSlider;
static bool sShowDebug;
static bool sIsIngame;

#include "client/renderer/block/model/BlockModel.h"
#include "resources/model/ModelBakery.h"

void testFunction() {
	ModelBakery_GetModel("block/grass_block");
}

void gInit(const char* name, bool isNew, bool demo) {
	sUsername = (char*)malloc(strlen(name) + 1);
	strcpy(sUsername, name);
	sIsNew3ds = isNew;
	sIsDemo	  = demo;

	// temporary
	gIsNew3ds = isNew;
}

static void deinit() {
	if (sHasExited)
		return;

	sHasExited = true;

	if (gWorld) {
		gReleaseWorld();
	}

	SaveManager_Deinit(&sSavemgr);

	Region_DeinitPools();

	Cube_DeinitVBOs();

	/*Sound_Quit(0);
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
*/
	DebugUI_Deinit();

	ChunkWorker_Deinit(&sChunkWorker);

	GameRenderer_Deinit();

	LanguageManager_Deinit();

	aptUnhook(&sAptHook);

	sino_exit();

	// ndspExit();

	romfsExit();

	gfxExit();
}

static void onApt(APT_HookType hook, void* param) {
	switch (hook) {
		case APTHOOK_ONSLEEP:
			if (gWorld)
				gDisplayPauseMenu();
			break;
			break;
		case APTHOOK_ONEXIT:
			deinit();
			break;

		default:
			break;
	}
}

static void init() {
	if (sHasInited)
		return;

	sHasInited = true;

	gfxInitDefault();
	osSetSpeedupEnable(true);  // Enable N3DS 804MHz operation, where available
	gfxSet3D(true);

	aptHook(&sAptHook, onApt, (void*)NULL);

	testFunction();

	Region_InitPools();

	SaveManager_InitFileSystem();

	ChunkWorker_Init(&sChunkWorker);
	ChunkWorker_AddHandler(&sChunkWorker, WorkerItemType_PolyGen, (WorkerFuncObj){ &PolyGen_GeneratePolygons, NULL, true });
	ChunkWorker_AddHandler(&sChunkWorker, WorkerItemType_BaseGen, (WorkerFuncObj){ &SuperFlatGen_Generate, &sFlatGen, true });
	ChunkWorker_AddHandler(&sChunkWorker, WorkerItemType_BaseGen, (WorkerFuncObj){ &SmeaGen_Generate, &smeaGen, true });

	sino_init();

	LanguageManager_Init();

	PlayerController_Init(&sPlayerCtrl);

	SuperFlatGen_Init(&sFlatGen);
	SmeaGen_Init(&smeaGen);

	GameRenderer_Init();

	DebugUI_Init();

	SaveManager_Init(&sSavemgr);
	ChunkWorker_AddHandler(&sChunkWorker, WorkerItemType_Load, (WorkerFuncObj){ &SaveManager_LoadChunk, &sSavemgr, true });
	ChunkWorker_AddHandler(&sChunkWorker, WorkerItemType_Save, (WorkerFuncObj){ &SaveManager_SaveChunk, &sSavemgr, true });

	// ndspInit();
	/*sBackgroundSound.background = true;
	sBackgroundSound.path		= String_ParsePackName(PACK_VANILLA, PATH_PACK_SOUNDS, "music/1.opus");
	Sound_PlayOpus(&sBackgroundSound);
*/
	Cube_InitVBOs();  // needs to be called AFTER all cubes have been built.

	sLastTime = svcGetSystemTick();

	Screen_SetScreen(SCREEN_TITLE);

#ifdef _DEBUG
	gSetShowDebug(true);
#else
	gSetShowDebug(false);
#endif

	aptSetHomeAllowed(true);
}

static void fetchInput() {
	circlePosition circlePos, cstickPos;
	touchPosition touchPos;

	hidScanInput();
	hidCircleRead(&circlePos);
	hidCstickRead(&cstickPos);
	hidTouchRead(&touchPos);

	gInputOld = gInput;
	gInput	  = (InputData){ hidKeysHeld(), hidKeysDown(), hidKeysUp(),	 circlePos.dx, circlePos.dy,
							 touchPos.px,	touchPos.py,   cstickPos.dx, cstickPos.dy };
}

static void processButtonBinds() {
}

static void runTick(float tickDt) {
	processButtonBinds();

	if (gWorld) {
		WorldRenderer_Tick();
	}

	PlayerController_Tick(&sPlayerCtrl, &sPlayerSound, tickDt);
}

static void runGameLoop() {
	sIsIngame = gWorld && gWorld->active && !currentScreen;

#ifdef _DEBUG
	if (sShowDebug) {
		DebugUI_Text("3DSCraft " APP_VERSION);
		float frameMult = (FPS + (FPS - sFps)) * 0.1f;
		DebugUI_Text("%d FPS T: 60 CPU: %.2f%% GPU: %.2f%% Buf: %.2f%% Lin: %loKB", sFps, C3D_GetProcessingTime() * frameMult,
					 C3D_GetDrawingTime() * frameMult, C3D_GetCmdBufUsage() * 100.f, linearSpaceFree() >> 10);

		DebugUI_TextAt(sIsIngame ? 24 : 29, "%u FPS\t\t\t\t\t\t\t         %u TPS", sFps, sTps);
	}
#endif

	sCurrentTime = svcGetSystemTick();
	sDt			 = ((float)(sCurrentTime / (float)CPU_TICKS_PER_MSEC) - (float)(sLastTime / (float)CPU_TICKS_PER_MSEC)) / 1000.f;
	sLastTime	 = sCurrentTime;
	sTimeAccum += sDt;

	sFrameCounter++;
	sFpsClock += sDt;
	if (sFpsClock >= 1.f) {
		sFps		  = sFrameCounter;
		sFrameCounter = 0;
		sFpsClock	  = 0.f;

		sTps		 = sTickCounter;
		sTickCounter = 0;
	}

	s3dSlider = osGet3DSliderState() * PLAYER_HALFEYEDIFF;

	if (currentScreenIdx != SCREEN_PAUSE) {
		if (gInput.keysdown & KEY_START && !currentScreen) {
			gDisplayPauseMenu();
		}
	}

	fetchInput();

	// OG Game has Simulations capped to 20 fps and render is X fps
	const float tickDt = sDt * (60 / 20);
	while (sTimeAccum >= 1.f / 20.f) {
		runTick(tickDt);

		sTimeAccum -= 1.f / 20.f;
		sTickCounter++;
	}
	if (gWorld) {
		PolyGen_Harvest();
	}

	GameRenderer_Render();

	if (currentScreen) {
		Screen_Tick();
	}
}

void gRun() {
	sRunning = true;

	init();

	while (sRunning && aptMainLoop()) {
		runGameLoop();
	}

	deinit();
}

void gReleaseWorld() {
	gWorld->active = false;

	SaveManager_Unload(&sSavemgr);

	for (int i = 0; i < CHUNKCACHE_SIZE; i++) {
		for (int j = 0; j < CHUNKCACHE_SIZE; j++) {
			World_UnloadChunk(gWorld->chunkCache[i][j]);
		}
	}
	ChunkWorker_Finish(&sChunkWorker);

	World_Deinit();
}

void gLoadWorld(char* path, char* name, WorldGenType worldType, bool newWorld) {
	// Crash("path: %s\nname: %s\nworlsDtype: %d\nnewWorld: %s", path, name, worldType, newWorld ? "true" : "false");
	char worldPath[256];
	sprintf(worldPath, PATH_SAVES "%s", path);

	World_Init();

	strcpy(gWorld->worldInfo.name, name);
	strcpy(gWorld->worldInfo.path, worldPath);
	gWorld->worldInfo.seed	 = 28112000;
	gWorld->genSettings.type = worldType;
	gWorld->active			 = true;

	GameRules_Reset(&gWorld->worldInfo.gamerules);

	SaveManager_Load(&sSavemgr);

	ChunkWorker_SetHandlerActive(&sChunkWorker, WorkerItemType_BaseGen, &sFlatGen, gWorld->genSettings.type == WorldGen_SuperFlat);
	ChunkWorker_SetHandlerActive(&sChunkWorker, WorkerItemType_BaseGen, &smeaGen, gWorld->genSettings.type == WorldGen_Default);

	gWorld->cacheTranslationX = WorldToChunkCoord(FastFloor(gPlayer->position.x));
	gWorld->cacheTranslationZ = WorldToChunkCoord(FastFloor(gPlayer->position.z));
	for (int i = 0; i < CHUNKCACHE_SIZE; ++i) {
		for (int j = 0; j < CHUNKCACHE_SIZE; ++j) {
			gWorld->chunkCache[i][j] = World_LoadChunk(i - (CHUNKCACHE_SIZE >> 1) + gWorld->cacheTranslationX,
													   j - (CHUNKCACHE_SIZE >> 1) + gWorld->cacheTranslationZ);
		}
	}

	for (int i = 0; i < 3; ++i) {
		while (sChunkWorker.working || gWorkqueue.queue.length > 0) {
			svcSleepThread(50000000);  // 1 Tick
		}
		World_UpdateChunkGen();
	}

	if (newWorld) {
		int highestblock = 0;
		for (int x = -1; x < 1; ++x) {
			for (int z = -1; z < 1; ++z) {
				int height = World_GetChunkHeight(x, z);
				if (height > highestblock)
					highestblock = height + 1;
			}
		}
		Player_SetPosBlock(0, highestblock, 0);
		gPlayer->spawnPos = gPlayer->position;
		gPlayer->hunger	  = 20;
		gPlayer->hp		  = 20;
	}
	Screen_SetScreen(SCREEN_NONE);
	sLastTime = svcGetSystemTick();	 // fix timing
}

void gStop() {
	sRunning = false;
}

void gSetScreen(u8 idx) {
	Screen_SetScreen(idx);
}

void gDisplayPauseMenu() {
	gSetScreen(SCREEN_PAUSE);
}

float gGet3dSlider() {
	return s3dSlider;
}

void gSetShowDebug(bool b) {
	sShowDebug = b;
}
bool gGetShowDebug() {
	return sShowDebug;
}

bool gIsRunning() {
	return sRunning;
}

bool gIngame() {
	return sIsIngame;
}