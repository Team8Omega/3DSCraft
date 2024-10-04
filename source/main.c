#include "client/Game.h"

#include <3ds.h>
#include <string.h>
#include <unistd.h>

#include "client/Crash.h"
#include "util/Paths.h"

void checkValid();
void getUsername();

static char *name;
static bool isNew;

#ifdef _DEMO_BUILD
static const bool isDemo = true;
#else
static const bool isDemo = false;
#endif

int main() {
	aptSetHomeAllowed(false);

	checkValid();

	getUsername();
	APT_CheckNew3DS(&isNew);

	gInit(name, isNew, isDemo);

	gRun();

	return 0;
}

#define PATHPACK_SHORT PATH_PACKS PACK_VANILLA "/"

void checkValid() {
	romfsInit();

	// Check for some files
	if (access(PATHPACK_SHORT PATH_PACK_TEXTURES "/block/stone.png", F_OK)	//
		|| access(PATHPACK_SHORT PATH_PACK_LANG "/en_us.mp", F_OK)			//
																			//|| access(PATHPACK_SHORT PATH_PACK_LANG "/en_us.mp", F_OK)
	)
		Crash(0,
			  "Hello there! Cannot find the vanilla game assets.\n\n\nPlease follow this link for instructions:\n"
			  "\'github.com/Team8Omega/3DSCraft-ResourcePacker\'\n\n\nYou have to "
			  "validly provide your own assets of thegame you purchased, as we do not support piracy.\n\nYou are not allowed to ask for\n"
			  "another persons dump, as we expect every player\nto have bought the original game.\nYou provide the assets, we provide the "
			  "code.\n\n\n\n\n\n\n3DSCraft by Team Omega, Minecraft by Mojang.\n\n\n\n");

	// Check for valid license
	if (access("romfs:/licenses.txt", F_OK))
		Crash(0,
			  "This build is shipped without license information for third parties, and is therefore not legit.\nPlease build with "
			  "\'licenses.txt\', and try again.\nBig thanks to all the people who openly\nprovide their code for people to share.");
}

void getUsername() {
	const u16 *block = malloc(0x1C);

	cfguInit();
	CFGU_GetConfigInfoBlk2(0x1C, 0xA0000, (u8 *)block);
	cfguExit();

	name		= malloc(0x14);
	ssize_t len = utf16_to_utf8((u8 *)name, block, 0x14);
	if (len <= 0)
		strcpy(name, "Steve");
	else
		name[len] = '\0';
}
