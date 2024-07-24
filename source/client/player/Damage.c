#include "client/player/Damage.h"

#include <3ds.h>

#include "client/player/DamageDef.h"
#include "client/player/Player.h"

void OvertimeDamage(Player* player) {
	if (player->dmg.cause == DAMAGECAUSE_FIRE) {
		while (time < 0) {
			player->hp -= 1;
			// sleep(1);
			svcSleepThread(1000000000);
			player->dmg.time -= 1;
		}
		// cause == NULL;
		// time == NULL;
	}
	if (player->dmg.cause == DAMAGECAUSE_POISON) {
		while (time < 0) {
			player->hp -= 1;
			// sleep(1);
			svcSleepThread(1000000000);
			player->dmg.time -= 1;
		}
	}
}
