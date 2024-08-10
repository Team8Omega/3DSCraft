#include "client/gui/IngameGui.h"

#include <string.h>

#include "client/player/Player.h"
#include "client/renderer/SpriteBatch.h"

void renderExpBar();
void renderHealth();

void IngameGui_RenderGameOverlay() {
	SpriteBatch_BindGuiTexture(GuiTexture_Icons);
	SpriteBatch_PushQuad(200 / 2 - 16 / 2, 120 / 2 - 16 / 2, 0, 16, 16, 0, 0, 16, 16);

	renderExpBar();
	renderHealth();
}

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
		u8 barLength = 182;
		u16 xpFill	 = (int)(gPlayer->experience * (float)(barLength + 1));

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