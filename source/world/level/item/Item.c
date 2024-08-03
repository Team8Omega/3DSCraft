#include "world/level/item/Item.h"

#include "client/model/VertexFmt.h"
#include "client/renderer/texture/TextureMap.h"

static Texture_Map textureMap;

// PATH PREFIX
#define PPRX "block/"

#define ITEM_TEXTURE_FILES A(totem, "dirt.png")

#define A(i, n) PPRX n
const char* item_texture_files[] = { ITEM_TEXTURE_FILES };
#undef A

// static struct { Texture_MapIcon totem; } icon;
Texture_MapIcon totem;

void Item_Init() {
	u16 icon = Texture_MapAdd("block/dirt.png");
	Texture_MapInit(&textureMap);
	totem = gTexMapBlock.icons[icon];
}
void Item_Deinit() {
	C3D_TexDelete(&textureMap.texture);
}

void* Item_GetTextureMap() {
	return &textureMap.texture;
}

void Item_GetTexture(Item item, Direction direction, u8 metadata, s16* out_uv) {
	Texture_MapIcon i = { 0, 0 };
	switch (item) {
		case Item_Totem:
			i = totem;
			break;
		default:
			break;
	}
	out_uv[0] = i.u;
	out_uv[1] = i.v;
}

const char* ItemNames[Items_Count] = { "Totem" };
