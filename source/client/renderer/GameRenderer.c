#include "client/renderer/GameRenderer.h"

#include <client/Camera.h>
#include <client/Crash.h>
#include <client/gui/DebugUI.h>
#include <client/gui/Gui.h>
#include <client/gui/GuiIngame.h>
#include <client/gui/Inventory.h>
#include <client/gui/screens/TitleScreen.h>
#include <client/renderer/Clouds.h>
#include <client/renderer/CubeMap.h>
#include <client/renderer/Cursor.h>
#include <client/renderer/PolyGen.h>
#include <client/renderer/Shader.h>
#include <client/renderer/SpriteBatch.h>
#include <client/renderer/WorldRenderer.h>
#include <client/renderer/texture/TextureMap.h>
#include <world/GrassColors.h>
#include <world/level/block/Block.h>

#include "client/gui/screens/PauseScreen.h"
#include "client/gui/screens/SelectWorldScreen.h"
#include "client/gui/screens/TitleScreen.h"

#include "gui_shbin.h"
#include "world_shbin.h"

#include <stdlib.h>

#define DISPLAY_TRANSFER_FLAGS                                                                                                             \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |        \
	 GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

#define CLEAR_COLOR_SKY 0x90d9ffff
// #define CLEAR_COLOR_SKY 0x06070cff
#define CLEAR_COLOR_BLACK 0x000000ff

static C3D_RenderTarget* renderTargets[2];
static C3D_RenderTarget* lowerScreen;

Shader shaderGui, shaderWorld, shaderWire;

static WorkQueue* workqueue;

void GameRenderer_Init(WorkQueue* queue) {
	workqueue = queue;

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	renderTargets[0] = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	renderTargets[1] = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	C3D_RenderTargetSetOutput(renderTargets[0], GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
	C3D_RenderTargetSetOutput(renderTargets[1], GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);

	lowerScreen = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	C3D_RenderTargetSetOutput(lowerScreen, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

	// World Shader
	Shader_Init(&shaderWorld, world_shbin, world_shbin_size, false);
	AttrInfo_Init(&shaderWorld.vertexAttribs);
	AttrInfo_AddLoader(&shaderWorld.vertexAttribs, 0, GPU_FLOAT, 3);
	AttrInfo_AddLoader(&shaderWorld.vertexAttribs, 1, GPU_SHORT, 2);
	AttrInfo_AddLoader(&shaderWorld.vertexAttribs, 2, GPU_UNSIGNED_BYTE, 3);
	AttrInfo_AddLoader(&shaderWorld.vertexAttribs, 3, GPU_UNSIGNED_BYTE, 3);
	// Gui Shader
	Shader_Init(&shaderGui, gui_shbin, gui_shbin_size, false);
	AttrInfo_Init(&shaderGui.vertexAttribs);
	AttrInfo_AddLoader(&shaderGui.vertexAttribs, 0, GPU_SHORT, 3);
	AttrInfo_AddLoader(&shaderGui.vertexAttribs, 1, GPU_SHORT, 3);

	PolyGen_Init();

	WorldRenderer_Init(gWorld.workqueue, shaderWorld.uLocProjection);

	SpriteBatch_Init(shaderGui.uLocProjection);

	Gui_Init();

	C3D_CullFace(GPU_CULL_BACK_CCW);

	Blocks_Init();

	GrassColors_Init();

	BiomeGenType_Init();

	Item_Init();

	ScreenManager_SetScreen(&sTitleScreen);

	TitleScreen_Init();
}

void GameRenderer_Deinit() {
	Item_Deinit();

	Texture_MapDeinit(&gTexMapBlock);

	PolyGen_Deinit();

	TitleScreen_Deinit();

	WorldRenderer_Deinit();

	Gui_Deinit();

	SpriteBatch_Deinit();

	Shader_Deinit(&shaderWorld);
	Shader_Deinit(&shaderGui);
	Shader_Deinit(&shaderWire);

	C3D_Fini();
}

void GameRenderer_Tick() {
	if (gWorld.active) {
		WorldRenderer_Tick();

		if (gInput.keysdown & KEY_START && !currentScreen) {
			ScreenManager_SetScreen(&sPauseScreen);
		}
	} else {
		CubeMap_Update(&gCamera.projection, f3_new(0.f, 0.003f, 0.f));
	}
	if (currentScreen) {
		ScreenManager_Update();
	}
}

void GameRenderer_Render() {
	float iod = osGet3DSliderState() * PLAYER_HALFEYEDIFF;

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

	if (gWorld.active) {
		PolyGen_Harvest();
	}

	for (int i = 0; i < 2; i++) {
		C3D_RenderTargetClear(renderTargets[i], C3D_CLEAR_ALL, gWorld.active ? CLEAR_COLOR_SKY : 0x000000FF, 0);

		C3D_FrameDrawOn(renderTargets[i]);

		SpriteBatch_StartFrame(400, 240);

		C3D_TexEnv* env = C3D_GetTexEnv(0);
		C3D_TexEnvInit(env);
		C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
		C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);

		Shader_Bind(&shaderWorld);

		if (gWorld.active) {
			C3D_TexBind(0, &gTexMapBlock.texture);
			WorldRenderer_Render(!i ? -iod : iod);

			if (!currentScreen)
				GuiIngame_RenderGameOverlay();
		} else {
			TitleScreen_DrawUp();
		}

		if (currentScreen)
			ScreenManager_DrawUp();

		Shader_Bind(&shaderGui);

		SpriteBatch_Render(GFX_TOP);

		if (iod <= 0.f)
			break;
	}

	C3D_RenderTargetClear(lowerScreen, C3D_CLEAR_ALL, CLEAR_COLOR_BLACK, 0);
	C3D_FrameDrawOn(lowerScreen);

	SpriteBatch_StartFrame(320, 240);

	if (currentScreen)
		ScreenManager_DrawDown();
	else {
		if (gWorld.active) {
			SpriteBatch_SetScale(2);
			gPlayer.quickSelectBarSlots = 9;
			Inventory_DrawQuickSelect(160 / 2 - Inventory_QuickSelectCalcWidth(gPlayer.quickSelectBarSlots) / 2,
									  120 - INVENTORY_QUICKSELECT_HEIGHT, gPlayer.quickSelectBar, gPlayer.quickSelectBarSlots,
									  &gPlayer.quickSelectBarSlot);
			gPlayer.inventorySite =
				Inventory_Draw(16, 0, 160, gPlayer.inventory, sizeof(gPlayer.inventory) / sizeof(ItemStack), gPlayer.inventorySite);
		}
	}

	DebugUI_Draw();

	SpriteBatch_Render(GFX_BOTTOM);

	C3D_FrameEnd(0);
}
