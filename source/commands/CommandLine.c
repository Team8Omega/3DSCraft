#include "commands/CommandLine.h"
#include "world/level/storage/SaveManager.h"

#include <3ds.h>

#include "client/gui/DebugUI.h"
#include <mpack/mpack.h>

#include <stdio.h>
#include <string.h>

void CommandLine_Activate() {
	static SwkbdState swkbd;
	static char textBuffer[64];
	swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, 64);
	swkbdSetHintText(&swkbd, "Enter command");

	int button = swkbdInputText(&swkbd, textBuffer, sizeof(textBuffer));
	if (button == SWKBD_BUTTON_CONFIRM) {
		CommandLine_Execute(textBuffer);
	}
}

void CommandLine_Execute(const char* text) {
	int length = strlen(text);
	mpack_writer_t writer;
	mpack_error_t err = mpack_writer_destroy(&writer);
	if (length >= 1 && text[0] == '/' && gPlayer.cheats == true) {
		if (length >= 9) {
			float x, y, z;
			if (sscanf(&text[1], "tp %f %f %f", &x, &y, &z) == 3) {
				gPlayer.position.x = x;
				gPlayer.position.y = y + 1;
				gPlayer.position.z = z;
				DebugUI_Log("Teleported to %f, %f %f", x, y, z);
			}
		}
		// int playerid;
		if (length == 2 && text[1] == 'k') {
			gPlayer.hp = 0;
			DebugUI_Log("Killed player");
		}
		int hp;
		if (sscanf(&text[1], "hp %i", &hp)) {
			if (hp > 0 && hp < 21) {
				gPlayer.hp = hp;
				DebugUI_Log("Set player hp to %i", hp);
			} else {
				DebugUI_Log("Cannot set hp to %i", hp);
			}
		}
		float sx, sy, sz;
		if (sscanf(&text[1], "ws %f %f %f", &sx, &sy, &sz) == 3) {
			gPlayer.spawnx	 = sx;
			gPlayer.spawny	 = sy;
			gPlayer.spawnz	 = sz;
			gPlayer.spawnset = 1;
			mpack_write_cstr(&writer, "sx");
			mpack_write_float(&writer, gPlayer.spawnx);
			mpack_write_cstr(&writer, "sy");
			mpack_write_float(&writer, gPlayer.spawny);
			mpack_write_cstr(&writer, "sz");
			mpack_write_float(&writer, gPlayer.spawnz);
			mpack_write_cstr(&writer, "ss");
			mpack_write_int(&writer, gPlayer.spawnset);
			DebugUI_Log("Set spawn to %f, %f %f", sx, sy, sz);
			if (err != mpack_ok) {
				DebugUI_Log("Mpack error %d while saving world manifest", err);
				DebugUI_Log("Save file possibly corrupted, don't hit me plz");
			}
		}
		int gm;
		if (sscanf(&text[1], "gm %i", &gm)) {
			if (gm > 0 && gm < 5) {
				gPlayer.gamemode = gm;
				DebugUI_Log("Set gamemode to %i", gm);
			} else {
				DebugUI_Log("Cannot set gamemode to %i", gm);
			}
		}
		int hunger;
		if (sscanf(&text[1], "hunger %i", &hunger)) {
			if (hunger > 0 && hunger < 5) {
				gPlayer.hunger = hunger;
				DebugUI_Log("Set hunger to %i", hunger);
			} else {
				DebugUI_Log("Cannot set hunger to %i", hunger);
			}
		}
		int diff;
		if (sscanf(&text[1], "diff %i", &diff)) {
			if (diff > 0 && diff < 6) {
				gPlayer.difficulty = diff;
				DebugUI_Log("Set difficulty to %i", diff);
			} else {
				DebugUI_Log("Cannot set difficulty to %i", diff);
			}
		}
		/*int bx, by, bz; 					idfk how to get it to read strings as arguments, compiler isn't liking it
		char block;
		if (sscanf(&text[1], "sb %i %i %i", &bx, &by, &bz) == 3 &&) {
			World_SetBlock(gPlayer.gWorld.bx,by,bz, block);
			DebugUI_Log("Block at %f, %f %f is now %c", bx, by, bz,block);
		}*/
	}
}
