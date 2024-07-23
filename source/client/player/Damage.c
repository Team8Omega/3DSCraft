#include "client/player/Damage.h"

#include <3ds.h>

#include "client/player/Player.h"

void OvertimeDamage(const char* cause, int time) {
	Player* player;
	if (cause == "fire") {
		while (time < 0) {
			player->hp - 1;
			//sleep(1);
			svcSleepThread(1000000000);
			time - 1;
		}
		cause == NULL;
		time == NULL;
	}
	if (cause == "poison") {
		while (time < 0) {
			player->hp - 1;
			// sleep(1);
			svcSleepThread(1000000000);
			time - 1;
		}
	}
}
