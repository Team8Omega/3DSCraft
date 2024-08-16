#include "client/3DSCraft.h"

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

void checkValid() {
	romfsInit();

	// Check for block asset
	if (access(PATH_PACKS PACK_VANILLA "/" PATH_PACK_TEXTURES "/"
									   "block/stone.png",
			   F_OK))
		Crash(
			"Please provide assets, check\n \'github.com/Team8Omega/3DSCraft-ResourcePacker\'\nfor infos.\nYou have to validly provide "
			"your own assets of the game you purchased, as we do not support piracy.\n\nFILENOTFOUND: "
			"block/stone.png");

	// Check for valid license
	if (access("romfs:/licenses.txt", F_OK))
		Crash(
			"This build is shipped without license information for third parties, and is therefore not legit.\nPlease build with "
			"\'licenses.txt\', and try again.\nBig thanks to all the people who openly\nprovide their code for people to share.");
}