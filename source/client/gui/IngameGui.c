#include "client/gui/IngameGui.h"

#include <string.h>

#include "client/Minecraft.h"
#include "client/gui/Gui.h"
#include "client/player/Player.h"
#include "client/renderer/SpriteBatch.h"

// this is actual minecraft ported code

void renderHealth() {
	// basic implementation from Minecraft indev
	// TODO: more features will be added later#

	u8 health = gPlayer->hp;
	u16 yPos  = 120 - 21;
	SpriteBatch_BindGuiTexture(GuiTexture_Icons);
	for (u8 amount = 0; amount < 10; ++amount) {
		u8 var6	  = 0;
		bool var9 = true;
		if (var9) {
			var6 = 1;
		}

		u8 spriteSize = 9;

		u8 prevHealth = gPlayer->hp;
		// TODO: Y- korregieren
		if (health <= 4) {
			yPos += rand() % 2;
		}
		u8 width = (amount * 8);

		SpriteBatch_PushQuad(spriteSize + width, yPos, -1, spriteSize, spriteSize, 16 + var6 * spriteSize, 0, spriteSize, spriteSize);

		u8 healthVar = (amount << 1) + 1;

		if (var9) {
			SpriteBatch_PushQuad(spriteSize + (amount * 8), yPos, 0, spriteSize, spriteSize, healthVar < prevHealth ? 70 : 79, 0,
								 spriteSize, spriteSize);
		}

		SpriteBatch_PushQuad(spriteSize + (amount * 8), yPos, 0, spriteSize, spriteSize, healthVar < health ? 52 : 61, 0, spriteSize,
							 spriteSize);
	}
}

void renderExpBar() {
	// harcoded cap for now
	u16 barCap = 10;

	SpriteBatch_BindGuiTexture(GuiTexture_Icons);

	if (barCap > 0) {
#define barLength 182
		u16 xpFill = (int)(gPlayer->experience * (float)(barLength + 1));

		u8 y = 120 - 9;
		SpriteBatch_PushQuad(200 / 2 - 182 / 2, y, 0, barLength, 5, 0, 64, barLength, 5);

		if (xpFill > 0) {
			SpriteBatch_PushQuad(200 / 2 - 182 / 2, y, 1, xpFill, 5, 0, 69, xpFill, 5);
		}
	}

	if (gPlayer->experienceLevel > 0) {
		char experienceStr[20];	 // buffer to hold the string representation of experience level

		u16 experienceInt = (u16)gPlayer->experienceLevel;
		snprintf(experienceStr, sizeof(experienceStr), "%d", experienceInt);  // Format as integer

		u16 textWidth = SpriteBatch_CalcTextWidth(experienceStr);

		u8 textY = 10;

		SpriteBatch_PushText(200 / 2 - textWidth / 2 + 1, 120 - textY, 2, SHADER_RGB(0, 0, 0), false, INT_MAX, 0, experienceStr);
		SpriteBatch_PushText(200 / 2 - textWidth / 2 - 1, 120 - textY, 2, SHADER_RGB(0, 0, 0), false, INT_MAX, 0, experienceStr);
		SpriteBatch_PushText(200 / 2 - textWidth / 2, 120 - textY + 1, 2, SHADER_RGB(0, 0, 0), false, INT_MAX, 0, experienceStr);
		SpriteBatch_PushText(200 / 2 - textWidth / 2, 120 - textY - 1, 2, SHADER_RGB(0, 0, 0), false, INT_MAX, 0, experienceStr);
		SpriteBatch_PushText(200 / 2 - textWidth / 2, 120 - textY, 3, SHADER_RGB(100, 255, 32), false, INT_MAX, 0, experienceStr);
	}
}

void IngameGui_RenderTop() {
	SpriteBatch_BindGuiTexture(GuiTexture_Icons);
	SpriteBatch_PushQuad(200 / 2 - 16 / 2, 120 / 2 - 16 / 2, 0, 16, 16, 0, 0, 16, 16);

	renderExpBar();
	renderHealth();
}
enum {
	CLICKACTION_SCREENINVENTORY = 0,
	CLICKACTION_SCREENITEMS,
	CLICKACTION_SCREENDEBUG
};

typedef u8 IngameClickAction;

static void callAction(IngameClickAction a) {
	switch (a) {
		case CLICKACTION_SCREENINVENTORY:
			break;

		case CLICKACTION_SCREENITEMS:
			break;
	}
}

void IngameGui_RenderBottom() {
	SpriteBatch_SetScale(1);
	SpriteBatch_BindGuiTexture(GuiTexture_Widgets);

	// quick items
	SpriteBatch_PushQuad(0, 0, 0, 320, 41, 0, 0, 182, 23);

#define SELBOX_WIDTH 37
	u8 selNow = gPlayer->quickSelectBarSlot;
	SpriteBatch_PushQuad(1 + selNow * SELBOX_WIDTH, 1, 0, SELBOX_WIDTH, 41, 3, 24, 20, 22);

	// bg
	Gui_DrawBackgroundFull(0, -1);

	// bar below
	SpriteBatch_PushSingleColorQuad(0, 210, 0, 320, 30, SHADER_RGB(8, 8, 8));
	SpriteBatch_PushSingleColorQuad(0, 210, 1, 320, 1, SHADER_RGB(14, 14, 14));

	SpriteBatch_SetScale(2);

	// buttons
	if (Gui_Button(true, 130, 30, 30, 2, "INV"))
		callAction(CLICKACTION_SCREENINVENTORY);
	if (Gui_Button(true, 130, 50, 30, 2, "ITM"))
		callAction(CLICKACTION_SCREENITEMS);

	// held item text
	SpriteBatch_PushText(2, 110, 1, SHADER_RGB(18, 18, 18), false, INT_MAX, 0, "Helditemname");
}