#include "client/renderer/SpriteBatch.h"

#include "client/gui/DebugUI.h"
#include "client/gui/font/FontLoader.h"
#include "client/renderer/texture/TextureMap.h"
#include "core/VertexFmt.h"
#include "util/StringUtils.h"
#include "world/level/block/Block.h"
#include "world/level/block/states/BlockStates.h"

#include <stdarg.h>

#include <vec/vec.h>

typedef struct {
	int depth;
	C3D_Tex* texture;
	s16 x0, y0, x1, y1;	 // top left, right
	s16 x2, y2, x3, y3;	 // bottom left, right
	s16 u0, v0, u1, v1;
	s16 color;
} Sprite;

static vec_t(Sprite) cmdList;
static C3D_Tex* currentTexture = NULL;
static GuiVertex* vertexList[2];
static int projUniform;

static Font* font;
static C3D_Tex whiteTex;
static C3D_Tex widgetsTex;
static C3D_Tex iconsTex;
static C3D_Tex menuBackgroundTex;

static C3D_Mtx iconModelMtx;

static u8 screenIdx;
static const u8 screenHeight = GSP_SCREEN_WIDTH;  // its 240.
static u16 screenWidth		 = 0;
static int guiScale			 = 2;

void SpriteBatch_Init(int projUniform_) {
	vec_init(&cmdList);

	vertexList[0] = linearAlloc(sizeof(GuiVertex) * 256);
	vertexList[1] = linearAlloc(sizeof(GuiVertex) * 2 * (4096 + 1024));

	projUniform = projUniform_;

	font = (Font*)malloc(sizeof(Font));
	FontLoader_Init(font, String_ParsePackName(PACK_VANILLA, PATH_PACK_TEXTURES, "font/ascii.png"));
	Texture_Load(&widgetsTex, "gui/widgets.png");
	Texture_Load(&iconsTex, "gui/icons.png");
	u8 data[16 * 16];
	memset(data, 0xff, 16 * 16 * sizeof(u8));
	C3D_TexInit(&whiteTex, 16, 16, GPU_L8);
	C3D_TexLoadImage(&whiteTex, data, GPU_TEXFACE_2D, 0);

	Texture_Load(&menuBackgroundTex, "gui/options_background.png");

	Mtx_Identity(&iconModelMtx);
	Mtx_RotateY(&iconModelMtx, M_PI / 4.f, false);
	Mtx_RotateX(&iconModelMtx, M_PI / 6.f, false);
}
void SpriteBatch_Deinit() {
	vec_deinit(&cmdList);
	linearFree(vertexList[0]);
	linearFree(vertexList[1]);

	C3D_TexDelete(&font->texture);
	free(font);

	C3D_TexDelete(&whiteTex);
	C3D_TexDelete(&widgetsTex);
	C3D_TexDelete(&menuBackgroundTex);
}

void SpriteBatch_BindTexture(C3D_Tex* texture) {
	currentTexture = texture;
}
void SpriteBatch_BindGuiTexture(GuiTexture texture) {
	switch (texture) {
		case GuiTexture_Blank:
			currentTexture = &whiteTex;
			break;
		case GuiTexture_Font:
			currentTexture = &font->texture;
			break;
		case GuiTexture_Widgets:
			currentTexture = &widgetsTex;
			break;
		case GuiTexture_Icons:
			currentTexture = &iconsTex;
			break;
		case GuiTexture_MenuBackground:
			currentTexture = &menuBackgroundTex;
			break;
		default:
			break;
	}
}

void SpriteBatch_PushSingleColorQuad(s16 x, s16 y, s16 z, s16 w, s16 h, s16 color) {
	SpriteBatch_BindTexture(&whiteTex);
	SpriteBatch_PushQuadColor(x, y, z, w, h, 0, 0, 4, 4, color);
}
void SpriteBatch_PushQuad(s16 x, s16 y, s16 z, s16 w, s16 h, s16 rx, s16 ry, s16 rw, s16 rh) {
	SpriteBatch_PushQuadColor(x, y, z, w, h, rx, ry, rw, rh, INT16_MAX);
}
void SpriteBatch_PushQuadColor(s16 x, s16 y, s16 z, s16 w, s16 h, s16 rx, s16 ry, s16 rw, s16 rh, s16 color) {
	vec_push(&cmdList, ((Sprite){ z, currentTexture, x * guiScale, y * guiScale, (x + w) * guiScale, y * guiScale, x * guiScale,
								  (y + h) * guiScale, (x + w) * guiScale, (y + h) * guiScale, rx, ry, rx + rw, ry + rh, color }));
}

static float rot = 0.f;
extern const WorldVertex block_sides_lut[6 * 6];
// TODO: Größe konfigurierbar machen
void SpriteBatch_PushIcon(BlockId block, u8 metadata, s16 x, s16 y, s16 z) {
	WorldVertex vertices[6 * 6];
	memcpy(vertices, block_sides_lut, sizeof(block_sides_lut));
	for (int i = 0; i < 6; i++) {
		if (i != Direction_Top && i != Direction_South && i != Direction_West)
			continue;

		s16* iconUV = BLOCKSTATES[block].states[metadata].variants[0].model->vertex[i * 6].uv;

#define oneDivIconsPerRow (32768 / 8)
#define halfTexel (6)

		u8 color[3];
		Block_GetItemColor(BLOCKS[block], i, metadata, color);

		for (int j = 0; j < 5; j++) {
			int k	   = i * 6 + j;
			C3D_FVec p = FVec3_New((float)vertices[k].pos.x - 0.5f, (float)vertices[k].pos.y - 0.5f, (float)vertices[k].pos.z - 0.5f);
			C3D_FVec v = Mtx_MultiplyFVec3(&iconModelMtx, p);
			vertices[k].pos.x = FastFloor(v.x * 20.f * guiScale) + (x + 16) * guiScale;
			vertices[k].pos.y = -FastFloor(v.y * 20.f * guiScale) + (y + 16) * guiScale;  // invertieren auf der Y-Achse
		}

		WorldVertex bottomLeft	= vertices[i * 6 + 0];
		WorldVertex bottomRight = vertices[i * 6 + 1];
		WorldVertex topRight	= vertices[i * 6 + 2];
		WorldVertex topLeft		= vertices[i * 6 + 4];

		C3D_Tex* texture = &gTexMapBlock.texture;

		s16 color16 = SHADER_RGB(color[0] >> 3, color[1] >> 3, color[2] >> 3);
		if (i == Direction_South)
			color16 = SHADER_RGB_DARKEN(color16, 14);
		else if (i == Direction_West)
			color16 = SHADER_RGB_DARKEN(color16, 10);

#define unpackP(x) (x).pos.x, (x).pos.y
		vec_push(&cmdList, ((Sprite){ z, texture, topLeft.pos.x, topLeft.pos.y, topRight.pos.x, topRight.pos.y, bottomLeft.pos.x,
									  bottomLeft.pos.y, bottomRight.pos.x, bottomRight.pos.y, iconUV[0] / 256,
									  iconUV[1] / 256 + TEXTURE_TILESIZE, iconUV[0] / 256 + TEXTURE_TILESIZE, iconUV[1] / 256, color16 }));

#undef unpackP
	}
}

int SpriteBatch_PushText(int x, int y, int z, s16 color, bool shadow, int wrap, int* ySize, const char* fmt, ...) {
	va_list arg;
	va_start(arg, fmt);
	int length = SpriteBatch_PushTextVargs(x, y, z, color, shadow, wrap, ySize, fmt, arg);
	va_end(arg);
	return length;
}

int SpriteBatch_PushTextSingle(int x, int y, int z, s16 color, bool shadow, int wrap, int* ySize, const char* text) {
	SpriteBatch_BindTexture(&font->texture);
#define CHAR_WIDTH 8
#define TAB_SIZE 4

	int offsetX = 0;
	int offsetY = 0;

	int maxWidth = 0;

	u8 i = 0;

	while (text[i] != '\0') {
		bool implicitBreak = offsetX + font->fontWidth[(u8)text[i]] >= wrap;
		if (text[i] == '\n' || implicitBreak) {
			offsetY += CHAR_HEIGHT;
			maxWidth = MAX(maxWidth, offsetX);
			offsetX	 = 0;
			if (implicitBreak)
				--i;
		} else if (text[i] == '\t') {
			offsetX = ((offsetX / CHAR_WIDTH) / TAB_SIZE + 1) * TAB_SIZE * CHAR_WIDTH;
		} else {
			if (text[i] != ' ') {
				int texX = text[i] % 16 * 8, texY = text[i] / 16 * 8;
				SpriteBatch_PushQuadColor(x + offsetX, y + offsetY, z, 8, 8, texX, texY, 8, 8, color);
				if (shadow)
					SpriteBatch_PushQuadColor(x + offsetX + 1, y + offsetY + 1, z - 1, 8, 8, texX, texY, 8, 8, SHADER_RGB(10, 10, 10));
			}
			offsetX += font->fontWidth[(u8)text[i]];
		}
		++i;
	}

	maxWidth = MAX(maxWidth, offsetX);

	if (ySize != NULL && (*ySize = 1))
		*ySize = offsetY + CHAR_HEIGHT;

	return maxWidth;
}

int SpriteBatch_PushTextVargs(int x, int y, int z, s16 color, bool shadow, int wrap, int* ySize, const char* fmt, va_list arg) {
	char buffer[256];
	vsprintf(buffer, fmt, arg);
	return SpriteBatch_PushTextSingle(x, y, z, color, shadow, wrap, ySize, buffer);
}

int SpriteBatch_CalcTextWidth(const char* text, ...) {
	va_list args;
	va_start(args, text);
	int length = SpriteBatch_CalcTextWidthVargs(text, args);
	va_end(args);

	return length;
}
int SpriteBatch_CalcTextWidthSingle(const char* text) {
	u8 i = 0;

	int length	  = 0;
	int maxLength = 0;
	while (text[i] != '\0') {
		if (text[i] == '\n') {
			maxLength = MAX(maxLength, length);
			length	  = 0;
		} else
			length += font->fontWidth[(u8)text[i++]];
	}

	maxLength = MAX(maxLength, length);

	return maxLength;
}

int SpriteBatch_CalcTextWidthVargs(const char* text, va_list args) {
	char fmtedText[256];
	vsprintf(fmtedText, text, args);
	return SpriteBatch_CalcTextWidthSingle(fmtedText);
}

char* SpriteBatch_TextTruncate(const char* text, size_t length) {
	const char* ellipsis  = "...";
	size_t ellipsisLength = strlen(ellipsis);

	if (strlen(text) <= length) {
		return strdup(text);
	}

	size_t truncatedLength = length + ellipsisLength;

	char* truncated = (char*)malloc(truncatedLength + 1);

	strncpy(truncated, text, length);

	strcpy(truncated + length, ellipsis);

	truncated[truncatedLength] = '\0';

	return truncated;
}

static int compareDrawCommands(const void* a, const void* b) {
	Sprite* ga = ((Sprite*)a);
	Sprite* gb = ((Sprite*)b);

	return ga->depth == gb->depth ? gb->texture - ga->texture : gb->depth - ga->depth;
}

int SpriteBatch_GetWidth() {
	return screenWidth / guiScale;
}
int SpriteBatch_GetHeight() {
	return screenHeight / guiScale;
}

void SpriteBatch_SetScale(int scale) {
	guiScale = scale;
}
int SpriteBatch_GetScale() {
	return guiScale;
}
void SpriteBatch_SetScreen(bool isTop) {
	if (isTop) {
		screenWidth = GSP_SCREEN_HEIGHT_TOP;
		screenIdx	= 0;
	} else {
		screenWidth = GSP_SCREEN_HEIGHT_BOTTOM;
		screenIdx	= 1;
	}
}

void SpriteBatch_Render() {
	rot += M_PI / 60.f;
	vec_sort(&cmdList, &compareDrawCommands);

	C3D_Mtx projMtx;
	Mtx_OrthoTilt(&projMtx, 0.f, screenWidth, screenHeight, 0.f, 1.f, -1.f, false);

	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, projUniform, &projMtx);

	C3D_DepthTest(false, GPU_GREATER, GPU_WRITE_ALL);

	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
	C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);

	*C3D_GetTexEnv(1) = *C3D_GetTexEnv(2);

	GuiVertex* usedVertexList = vertexList[screenIdx];

	int verticesTotal = 0;

	size_t vtx = 0;
	while (cmdList.length > 0) {
		size_t vtxStart = vtx;

		C3D_Tex* texture = vec_last(&cmdList).texture;
		float divW = 1.f / texture->width * INT16_MAX, divH = 1.f / texture->height * INT16_MAX;

		while (cmdList.length > 0 && vec_last(&cmdList).texture == texture) {
			Sprite cmd = vec_pop(&cmdList);
			s16 color  = cmd.color;

			s16 u0 = (s16)((float)cmd.u0 * divW), v0 = (s16)((float)cmd.v0 * divH);
			s16 u1 = (s16)((float)cmd.u1 * divW), v1 = (s16)((float)cmd.v1 * divH);

			usedVertexList[vtx++] = (GuiVertex){ { cmd.x3, cmd.y3, 0 }, { u1, v1, color } };
			usedVertexList[vtx++] = (GuiVertex){ { cmd.x1, cmd.y1, 0 }, { u1, v0, color } };
			usedVertexList[vtx++] = (GuiVertex){ { cmd.x0, cmd.y0, 0 }, { u0, v0, color } };

			usedVertexList[vtx++] = (GuiVertex){ { cmd.x0, cmd.y0, 0 }, { u0, v0, color } };
			usedVertexList[vtx++] = (GuiVertex){ { cmd.x2, cmd.y2, 0 }, { u0, v1, color } };
			usedVertexList[vtx++] = (GuiVertex){ { cmd.x3, cmd.y3, 0 }, { u1, v1, color } };
		}

		C3D_TexBind(0, texture);

		C3D_BufInfo* bufInfo = C3D_GetBufInfo();
		BufInfo_Init(bufInfo);
		BufInfo_Add(bufInfo, usedVertexList + vtxStart, sizeof(GuiVertex), 2, 0x10);

		C3D_DrawArrays(GPU_TRIANGLES, 0, vtx - vtxStart);

		verticesTotal += vtx - vtxStart;
	}
	C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_ALL);

	currentTexture = NULL;
	guiScale	   = 2;
}
