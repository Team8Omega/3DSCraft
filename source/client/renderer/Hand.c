#include "client/renderer/Hand.h"

#include "client/model/VertexFmt.h"
#include "util/math/NumberUtils.h"

#include "client/renderer/texture/TextureMap.h"
#include "world/level/block/Block.h"

#include <string.h>

static WorldVertex* handVBO;
static C3D_Tex SkinTexture;

extern const WorldVertex block_sides_lut[6 * 6];

void Hand_Init() {
	handVBO = linearAlloc(sizeof(block_sides_lut));

	Texture_Load(&SkinTexture, "entity/player/wide/steve.png");
}
void Hand_Deinit() {
	linearFree(handVBO);
	C3D_TexDelete(&SkinTexture);
}

void Hand_Draw(int projUniform, C3D_Mtx* projection, ItemStack stack) {
	C3D_Mtx pm;
	C3D_Mtx model;
	Mtx_Identity(&model);
	Mtx_Translate(&model, 0.5f + sinf(gPlayer->bobbing) * 0.03f + (stack.amount == 0) * 0.2f, -0.68f + ABS(sinf(gPlayer->bobbing)) * 0.01f,
				  -1.2f - (stack.amount == 0) * 0.4f, true);
	Mtx_RotateX(&model, M_PI / 18.f + (stack.amount == 0) * M_PI / 5.f, true);
	Mtx_RotateY(&model, M_PI / 18.f, true);
	if (gPlayer->breakPlaceTimeout > -0.1f) {
		float dist = sinf((gPlayer->breakPlaceTimeout + 0.1f) / (PLAYER_PLACE_REPLACE_TIMEOUT + 0.1f) * M_PI);
		float h	   = cosf((gPlayer->breakPlaceTimeout + 0.1f) / (PLAYER_PLACE_REPLACE_TIMEOUT + 0.1f) * M_PI);
		Mtx_RotateX(&model, -dist, true);
		Mtx_Translate(&model, 0.f, -h * 0.3f, -dist * 0.25f, true);
	}
	if (stack.amount == 0)
		Mtx_RotateZ(&model, DEG_TO_RAD * 40.f, true);
	Mtx_Scale(&model, 0.28f, 0.28f, stack.amount == 0 ? 0.7f : 0.28f);

	Mtx_Multiply(&pm, projection, &model);

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projUniform, &pm);

	memcpy(handVBO, block_sides_lut, sizeof(block_sides_lut));
	for (int i = 0; i < 6; i++) {
		if (stack.amount > 0) {
			C3D_TexBind(0, &gTexMapBlock.texture);

			s16 iconUV[2];
			u8 color[3];
			Block_GetBlockTexture(BLOCKS[stack.block], i, 0, 0, 0, stack.meta, iconUV);
			Block_GetItemColor(BLOCKS[stack.block], i, stack.meta, color);

#define oneDivIconsPerRow (32768 / 8)
#define halfTexel (6)

			for (int j = 0; j < 6; j++) {
				int idx = i * 6 + j;

				handVBO[idx].uv0[0] = (handVBO[idx].uv0[0] == 1 ? (oneDivIconsPerRow - 1) : 1) + iconUV[0];
				handVBO[idx].uv0[1] = (handVBO[idx].uv0[1] == 1 ? (oneDivIconsPerRow - 1) : 1) + iconUV[1];

				handVBO[idx].rgb[0] = color[0];
				handVBO[idx].rgb[1] = color[1];
				handVBO[idx].rgb[2] = color[2];
			}
		} else {
			C3D_TexBind(0, &SkinTexture);

			if (i == Direction_East || i == Direction_West) {  // eines der dümmsten Dinge, die ich jemals in meinem Leben getan habe
				const s16 uvRotationTable[2][2][2][2] = {
					{ { { 0, 1 }, { 0, 0 } }, { { 1, 1 }, { 1, 0 } } },
					{ { { 1, 0 }, { 1, 1 } }, { { 0, 0 }, { 0, 1 } } },
				};
				for (int j = 0; j < 6; j++) {
					int idx				= i * 6 + j;
					int u				= handVBO[idx].uv0[0];
					int v				= handVBO[idx].uv0[1];
					handVBO[idx].uv0[0] = uvRotationTable[(i == Direction_West) ? 1 : 0][v][u][0];
					handVBO[idx].uv0[1] = uvRotationTable[(i == Direction_East) ? 1 : 0][v][u][1];
				}
			}
			for (int j = 0; j < 6; j++) {
				int idx = i * 6 + j;
#define toTexCoord(x, tw) (s16)(((float)(x) / (float)(tw)) * (float)(1 << 15) - 1)
				const s16 uvLookUp[6][4] = {
					{ toTexCoord(48, 64), toTexCoord(52, 64), toTexCoord(20, 64), toTexCoord(32, 64) },	 // west = inside
					{ toTexCoord(40, 64), toTexCoord(44, 64), toTexCoord(20, 64), toTexCoord(32, 64) },	 // east = outside
					{ toTexCoord(52, 64), toTexCoord(56, 64), toTexCoord(20, 64), toTexCoord(32, 64) },	 // bottom = back
					{ toTexCoord(44, 64), toTexCoord(48, 64), toTexCoord(20, 64), toTexCoord(32, 64) },	 // top = front
					{ toTexCoord(48, 64), toTexCoord(52, 64), toTexCoord(16, 64), toTexCoord(20, 64) },	 // south = bottom
					{ toTexCoord(44, 64), toTexCoord(48, 64), toTexCoord(16, 64), toTexCoord(20, 64) },	 // north = top
				};

				handVBO[idx].uv0[0] = uvLookUp[i][handVBO[idx].uv0[0]];
				handVBO[idx].uv0[1] = uvLookUp[i][handVBO[idx].uv0[1] + 2];
			}
		}
	}

	C3D_AlphaTest(true, GPU_GEQUAL, 255);

	C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, handVBO, sizeof(WorldVertex), 4, 0x3210);

	C3D_DrawArrays(GPU_TRIANGLES, 0, 6 * 6);

	C3D_AlphaTest(false, GPU_GREATER, 0);
}
