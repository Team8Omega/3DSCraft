#include "client/Game.h"

#include <3ds.h>
#include <string.h>
#include <unistd.h>

#include "client/Crash.h"
#include "util/Paths.h"

void checkValid();

int main() {
	aptSetHomeAllowed(false);

	checkValid();

	ptmuInit();
	char nickname[0x16] = { 0 };
	CFGU_GetConfigInfoBlk2(0x16, 0x000A0000, nickname);
	ptmuExit();

	bool isNew;
	APT_CheckNew3DS(&isNew);

#ifdef _DEMO_BUILD
	const bool isDemo = true;
#else
	const bool isDemo = false;
#endif

	gInit(nickname, isNew, isDemo);

	gRun();

	return 0;
}

#define PATHPACK_SHORT PATH_PACKS PACK_VANILLA "/"

void checkValid() {
	romfsInit();

	// Check for block asset
	if (access(PATHPACK_SHORT PATH_PACK_TEXTURES "/block/stone.png", F_OK)	//
		|| access(PATHPACK_SHORT PATH_PACK_LANG "/en_us.mp", F_OK)			//
																			//|| access(PATHPACK_SHORT PATH_PACK_LANG "/en_us.mp", F_OK)
	)
		Crash(
			"Hello there! Cannot find the vanilla game assets.\n\n\nPlease follow this link for instructions:\n"
			"\'github.com/Team8Omega/3DSCraft-ResourcePacker\'\n\n\nYou have to "
			"validly provide your own assets of thegame you purchased, as we do not support piracy.\n\nYou are not allowed to ask for\n"
			"another persons dump, as we expect every player\nto have bought the original game.\nYou provide the assets, we provide the "
			"code.\n\n\n\n\n\n\n3DSCraft by Team Omega, Minecraft by Mojang.\n\n\n\n");

	// Check for valid license
	if (access("romfs:/licenses.txt", F_OK))
		Crash(
			"This build is shipped without license information for third parties, and is therefore not legit.\nPlease build with "
			"\'licenses.txt\', and try again.\nBig thanks to all the people who openly\nprovide their code for people to share.");
}
