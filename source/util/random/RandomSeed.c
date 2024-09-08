#include "util/random/RandomSeed.h"

#include <3ds.h>

static RandomSeed SEED_UNIQUIFIER = 8682522807148012ULL;

RandomSeed RandomSeed_Gen() {
	static s32 lock = 0;
	RandomSeed seed;

	syncArbitrateAddress(&lock, ARBITRATION_WAIT_IF_LESS_THAN, 0);

	seed			= SEED_UNIQUIFIER;
	SEED_UNIQUIFIER = seed * 1181783497276652981ULL;

	lock = 1;

	return seed ^ svcGetSystemTick();
}