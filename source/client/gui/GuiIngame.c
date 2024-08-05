#include "client/gui/GuiIngame.h"

#include <string.h>

#include "client/player/Player.h"
#include "client/renderer/SpriteBatch.h"

void renderExpBar();
void renderHealth();

void GuiIngame_RenderGameOverlay() {
	SpriteBatch_BindGuiTexture(GuiTexture_Icons);
	SpriteBatch_PushQuad(200 / 2 - 16 / 2, 120 / 2 - 16 / 2, 0, 16, 16, 0, 0, 16, 16);

	renderExpBar();
	renderHealth();
}

// this is actual minecraft ported code

void renderHealth() {
	// basic implementation from Minecraft indev
	// TODO: more features will be added later#

	int health = gPlayer.hp;
	int yPos   = 120 - 21;
	SpriteBatch_BindGuiTexture(GuiTexture_Icons);
	for (int amount = 0; amount < 10; ++amount) {
		int var6  = 0;
		bool var9 = true;
		if (var9) {
			var6 = 1;
		}

		int spriteSize = 9;

		int prevHealth = gPlayer.hp;
		// TODO: Y- korregieren
		if (health <= 4) {
			yPos += rand() % 2;
		}
		u8 width = (amount * 8);

		SpriteBatch_PushQuad(spriteSize + width, yPos, -1, spriteSize, spriteSize, 16 + var6 * spriteSize, 0, spriteSize, spriteSize);

		if (var9) {
			if ((amount << 1) + 1 < prevHealth) {
				SpriteBatch_PushQuad(spriteSize + (amount * 8), yPos, 0, spriteSize, spriteSize, 70, 0, spriteSize, spriteSize);
			}

			if ((amount << 1) + 1 == prevHealth) {
				SpriteBatch_PushQuad(spriteSize + (amount * 8), yPos, 0, spriteSize, spriteSize, 79, 0, spriteSize, spriteSize);
			}
		}

		if ((amount << 1) + 1 < health) {
			SpriteBatch_PushQuad(spriteSize + (amount * 8), yPos, 0, spriteSize, spriteSize, 52, 0, spriteSize, spriteSize);
		}

		if ((amount << 1) + 1 == health) {
			SpriteBatch_PushQuad(spriteSize + (amount * 8), yPos, 0, spriteSize, spriteSize, 61, 0, spriteSize, spriteSize);
		}
	}
}

void renderExpBar() {
	// harcoded cap for now
	int barCap = 10;

	SpriteBatch_BindGuiTexture(GuiTexture_Icons);

	if (barCap > 0) {
		int barLength = 182;
		int xpFill	  = (int)(gPlayer.experience * (float)(barLength + 1));

		int y = 120 - 9;
		SpriteBatch_PushQuad(200 / 2 - 182 / 2, y, 0, barLength, 5, 0, 64, barLength, 5);

		if (xpFill > 0) {
			SpriteBatch_PushQuad(200 / 2 - 182 / 2, y, 1, xpFill, 5, 0, 69, xpFill, 5);
		}
	}

	if (gPlayer.experienceLevel > 0) {
		char experienceStr[20];	 // buffer to hold the string representation of experience level

		int experienceInt = (int)gPlayer.experienceLevel;
		snprintf(experienceStr, sizeof(experienceStr), "%d", experienceInt);  // Format as integer

		int textWidth = SpriteBatch_CalcTextWidth(experienceStr);

		int textY = 10;

		SpriteBatch_PushText(200 / 2 - textWidth / 2 + 1, 120 - textY, 2, SHADER_RGB(0, 0, 0), false, INT_MAX, 0, experienceStr);
		SpriteBatch_PushText(200 / 2 - textWidth / 2 - 1, 120 - textY, 2, SHADER_RGB(0, 0, 0), false, INT_MAX, 0, experienceStr);
		SpriteBatch_PushText(200 / 2 - textWidth / 2, 120 - textY + 1, 2, SHADER_RGB(0, 0, 0), false, INT_MAX, 0, experienceStr);
		SpriteBatch_PushText(200 / 2 - textWidth / 2, 120 - textY - 1, 2, SHADER_RGB(0, 0, 0), false, INT_MAX, 0, experienceStr);
		SpriteBatch_PushText(200 / 2 - textWidth / 2, 120 - textY, 3, SHADER_RGB(100, 255, 32), false, INT_MAX, 0, experienceStr);
	}
}