#pragma once

#include "world/level/item/ItemStack.h"

#include "util/math/NumberUtils.h"

#define INVENTORY_QUICKSELECT_MAXSLOTS 9
#define INVENTORY_QUICKSELECT_HEIGHT (22 + 1)

#define INVENTORY_MAX_PER_SITE (32)

static inline int Inventory_QuickSelectCalcSlots(int screenwidth) {
	return MIN(INVENTORY_QUICKSELECT_MAXSLOTS, (screenwidth - 21 * 2) / 20 + 2);
}
static inline int Inventory_QuickSelectCalcWidth(int slots) {
	return 54 + (slots - 2) * 20;
}

void Inventory_DrawQuickSelect(int x, int y, ItemStack* stacks, u8 count, u8* selected);

u8 Inventory_Draw(int x, int y, int w, ItemStack* stacks, u8 count, u8 site);
