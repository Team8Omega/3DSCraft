#include "client/renderer/Renderer.h"

#include <client/Camera.h>
#include <client/gui/DebugUI.h>
#include <client/gui/Gui.h>
#include <client/gui/Inventory.h>
#include <client/renderer/Clouds.h>
#include <client/renderer/CubeMap.h>
#include <client/renderer/Cursor.h>
#include <client/renderer/PolyGen.h>
#include <client/renderer/WorldRenderer.h>
#include <client/renderer/texture/SpriteBatch.h>
#include <client/renderer/texture/TextureMap.h>
#include <world/level/block/Block.h>

#include <citro3d.h>

#include <client/gui/screens/SelectWorldScreen.h>
#include <client/gui/screens/TitleScreen.h>

#include <gui_shbin.h>
#include <world_shbin.h>

#define DISPLAY_TRANSFER_FLAGS                                                                                                             \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) |        \
	 GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

#define CLEAR_COLOR_SKY 0x90d9ffff
// #define CLEAR_COLOR_SKY 0x06070cff
#define CLEAR_COLOR_BLACK 0x000000ff

static C3D_RenderTarget* renderTargets[2];
static C3D_RenderTarget* lowerScreen;

static DVLB_s *world_dvlb, *gui_dvlb;
static shaderProgram_s world_shader, gui_shader;
static int world_shader_uLocProjection, gui_shader_uLocProjection;

static C3D_AttrInfo world_vertexAttribs, gui_vertexAttribs;

static C3D_Tex logoTex;

static World* world;
static Player* player;
static WorkQueue* workqueue;

static GameState* gamestate;

static state_machine_t* machine;

extern bool showDebugInfo;

extern Camera camera;

void Renderer_RenderGameOverlay();
void renderExpBar();

void Renderer_Init(World* world_, Player* player_, WorkQueue* queue, GameState* gamestate_) {
	machine = state_machine_create();
	state_machine_set_current_state(machine, TitleScreen);

	world	  = world_;
	player	  = player_;
	workqueue = queue;
	gamestate = gamestate_;

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

	renderTargets[0] = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	renderTargets[1] = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	C3D_RenderTargetSetOutput(renderTargets[0], GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
	C3D_RenderTargetSetOutput(renderTargets[1], GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);

	lowerScreen = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	C3D_RenderTargetSetOutput(lowerScreen, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

	world_dvlb = DVLB_ParseFile((u32*)world_shbin, world_shbin_size);
	shaderProgramInit(&world_shader);
	shaderProgramSetVsh(&world_shader, &world_dvlb->DVLE[0]);
	world_shader_uLocProjection = shaderInstanceGetUniformLocation(world_shader.vertexShader, "projection");

	gui_dvlb = DVLB_ParseFile((u32*)gui_shbin, gui_shbin_size);
	shaderProgramInit(&gui_shader);
	shaderProgramSetVsh(&gui_shader, &gui_dvlb->DVLE[0]);
	gui_shader_uLocProjection = shaderInstanceGetUniformLocation(gui_shader.vertexShader, "projection");

	AttrInfo_Init(&world_vertexAttribs);
	AttrInfo_AddLoader(&world_vertexAttribs, 0, GPU_FLOAT, 3);
	AttrInfo_AddLoader(&world_vertexAttribs, 1, GPU_SHORT, 2);
	AttrInfo_AddLoader(&world_vertexAttribs, 2, GPU_UNSIGNED_BYTE, 3);
	AttrInfo_AddLoader(&world_vertexAttribs, 3, GPU_UNSIGNED_BYTE, 3);

	AttrInfo_Init(&gui_vertexAttribs);
	AttrInfo_AddLoader(&gui_vertexAttribs, 0, GPU_SHORT, 3);
	AttrInfo_AddLoader(&gui_vertexAttribs, 1, GPU_SHORT, 3);

	PolyGen_Init(world, player_);

	CubeMap_Init(world_shader_uLocProjection);

	WorldRenderer_Init(player, world, workqueue, world_shader_uLocProjection);

	SpriteBatch_Init(gui_shader_uLocProjection);

	Gui_Init();

	C3D_CullFace(GPU_CULL_BACK_CCW);

	Block_Init();

	Item_Init();

	Texture_Load(&logoTex, "gui/title/minecraft.png");

	CubeMap_Set("gui/title/background/panorama", f3_new(0.f, 0.f, 0.f));
}

void Renderer_Deinit() {
	state_machine_delete(machine);

	C3D_TexDelete(&logoTex);

	Item_Deinit();

	Block_Deinit();

	PolyGen_Deinit();

	CubeMap_Deinit();

	WorldRenderer_Deinit();

	Gui_Deinit();

	SpriteBatch_Deinit();

	shaderProgramFree(&gui_shader);
	DVLB_Free(gui_dvlb);
	shaderProgramFree(&world_shader);
	DVLB_Free(world_dvlb);

	C3D_Fini();
}

void Renderer_Render() {
	float iod = osGet3DSliderState() * PLAYER_HALFEYEDIFF;

	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

	if (*gamestate == GameState_Playing)
		PolyGen_Harvest();
	else
		CubeMap_Update(&camera.projection, f3_new(0.f, 0.0013f, 0.f));

	for (int i = 0; i < 2; i++) {
		if (*gamestate == GameState_Playing) {
			C3D_RenderTargetClear(renderTargets[i], C3D_CLEAR_ALL, CLEAR_COLOR_SKY, 0);
		} else {
			C3D_RenderTargetClear(renderTargets[i], C3D_CLEAR_ALL, 0x000000FF, 0);
		}

		C3D_FrameDrawOn(renderTargets[i]);

		SpriteBatch_StartFrame(400, 240);

		C3D_TexEnv* env = C3D_GetTexEnv(0);
		C3D_TexEnvInit(env);
		C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
		C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);

		C3D_BindProgram(&world_shader);
		C3D_SetAttrInfo(&world_vertexAttribs);

		if (*gamestate == GameState_Playing) {
			C3D_TexBind(0, Block_GetTextureMap());

			WorldRenderer_Render(!i ? -iod : iod);

			Renderer_RenderGameOverlay();

		} else {
			CubeMap_Draw();

			SpriteBatch_SetScale(2);

			SpriteBatch_BindTexture(&logoTex);

			SpriteBatch_PushQuad(36, 35, 0, 128, 32, 0, 0, 1024, 256);

			SpriteBatch_PushText(0, 112, 0, INT16_MAX, false, INT_MAX, NULL, APP_VERSION);

			SpriteBatch_PushText(210 - 70, 112, 0, INT16_MAX, false, INT_MAX, NULL, AUTHOR);

			SpriteBatch_SetScale(1);
		}

		C3D_BindProgram(&gui_shader);
		C3D_SetAttrInfo(&gui_vertexAttribs);

		SpriteBatch_Render(GFX_TOP);

		if (iod <= 0.f)
			break;
	}

	C3D_RenderTargetClear(lowerScreen, C3D_CLEAR_ALL, CLEAR_COLOR_BLACK, 0);
	C3D_FrameDrawOn(lowerScreen);

	SpriteBatch_StartFrame(320, 240);

	if (*gamestate == GameState_SelectWorld) {
		state_machine_run(machine);
	} else {
		SpriteBatch_SetScale(2);
		player->quickSelectBarSlots = 9;
		Inventory_DrawQuickSelect(160 / 2 - Inventory_QuickSelectCalcWidth(player->quickSelectBarSlots) / 2,
								  120 - INVENTORY_QUICKSELECT_HEIGHT, player->quickSelectBar, player->quickSelectBarSlots,
								  &player->quickSelectBarSlot);
		player->inventorySite =
			Inventory_Draw(16, 0, 160, player->inventory, sizeof(player->inventory) / sizeof(ItemStack), player->inventorySite);
	}

	if (showDebugInfo)
		DebugUI_Draw();

	SpriteBatch_Render(GFX_BOTTOM);

	C3D_FrameEnd(0);
}

void Renderer_RenderGameOverlay() {
	SpriteBatch_BindGuiTexture(GuiTexture_Icons);
	SpriteBatch_PushQuad(200 / 2 - 16 / 2, 120 / 2 - 16 / 2, 0, 16, 16, 0, 0, 16, 16);

	renderExpBar();
}

// this is actual minecraft ported code

void renderExpBar() {
	// harcoded cap for now
	int barCap = 10;

	SpriteBatch_BindGuiTexture(GuiTexture_Icons);

	if (barCap > 0) {
		int barLength = 182;
		int xpFill	  = (int)(player->experience * (float)(barLength + 1));

		int y = 120 - 8;
		SpriteBatch_PushQuad(200 / 2 - 182 / 2, y, 0, barLength, 5, 0, 64, barLength, 5);

		if (xpFill > 0) {
			SpriteBatch_PushQuad(200 / 2 - 182 / 2, y, 1, xpFill, 5, 0, 69, xpFill, 5);
		}
	}

	if (player->experienceLevel > 0) {
		char experienceStr[20];	 // buffer to hold the string representation of experience level

		int experienceInt = (int)player->experienceLevel;
		snprintf(experienceStr, sizeof(experienceStr), "%d", experienceInt);  // Format as integer

		int textWidth = SpriteBatch_CalcTextWidth(experienceStr);

		SpriteBatch_PushText(200 / 2 - textWidth / 2 + 1, 120 - 17, 2, SHADER_RGB(0, 0, 0), false, INT_MAX, 0, experienceStr);
		SpriteBatch_PushText(200 / 2 - textWidth / 2 - 1, 120 - 17, 2, SHADER_RGB(0, 0, 0), false, INT_MAX, 0, experienceStr);
		SpriteBatch_PushText(200 / 2 - textWidth / 2, 120 - 17 + 1, 2, SHADER_RGB(0, 0, 0), false, INT_MAX, 0, experienceStr);
		SpriteBatch_PushText(200 / 2 - textWidth / 2, 120 - 17 - 1, 2, SHADER_RGB(0, 0, 0), false, INT_MAX, 0, experienceStr);
		SpriteBatch_PushText(200 / 2 - textWidth / 2, 120 - 17, 3, SHADER_RGB(100, 255, 32), false, INT_MAX, 0, experienceStr);
	}
}
