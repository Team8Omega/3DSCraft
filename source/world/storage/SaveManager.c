#include "world/storage/SaveManager.h"

#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include "client/Crash.h"
#include "client/gui/DebugUI.h"
#include "util/Paths.h"
#include "world/GameRules.h"

#define mkdirFlags S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH

void SaveManager_InitFileSystem() {
	mkdir(PATH_ROOT, mkdirFlags);
	mkdir(PATH_SAVES, mkdirFlags);
}

void SaveManager_Init(SaveManager* mgr) {
	vec_init(&mgr->regions);
}
void SaveManager_Deinit(SaveManager* mgr) {
	vec_deinit(&mgr->regions);
}

void SaveManager_Load(SaveManager* mgr) {
	// char buffer[256];
	// sprintf(buffer, "%s", gWorld->worldInfo.path);

	if (access(PATH_SAVES, F_OK))
		mkdir(PATH_SAVES, mkdirFlags);

	mkdir(gWorld->worldInfo.path, mkdirFlags);
	if (access(gWorld->worldInfo.path, F_OK))
		Crash("World path invalid, internal error. Cannot create world folder.\n\nPath: %s", gWorld->worldInfo.path);

	chdir(gWorld->worldInfo.path);

	mkdir("regions", mkdirFlags);

	if (access("level.mp", F_OK) != -1) {
		mpack_tree_t levelTree;
		mpack_tree_init_file(&levelTree, "level.mp", 0);
		mpack_node_t root = mpack_tree_root(&levelTree);

		mpack_node_copy_utf8_cstr(mpack_node_map_cstr(root, "name"), gWorld->worldInfo.name, sizeof(gWorld->worldInfo.name));

		mpack_node_t player = save_getMapArray(root, "players", 0);

		float x = save_get(player, float, "x", 0);
		float y = save_get(player, float, "y", 0) + 0.1f;
		float z = save_get(player, float, "z", 0);
		Player_SetPosWorld(f3_new(x, y, z));

		gPlayer->spawnPos.x = save_get(player, float, "sx", 0);
		gPlayer->spawnPos.y = save_get(player, float, "sy", 0);
		gPlayer->spawnPos.z = save_get(player, float, "sz", 0);

		gPlayer->pitch = save_get(player, float, "pitch", 0.f);
		gPlayer->yaw   = save_get(player, float, "yaw", 0.f);

		gPlayer->hp		  = save_get(player, u8, "hp", 20);
		gPlayer->hunger	  = save_get(player, u8, "hunger", 20);
		gPlayer->gamemode = save_get(player, u8, "gamemode", Gamemode_Survival);

		gPlayer->flying	   = save_get(player, bool, "flying", false);
		gPlayer->crouching = save_get(player, bool, "crouching", false);
		gPlayer->cheats	   = save_get(player, bool, "cheats", true);

		GameRules_Deserialize(&gWorld->worldInfo.gamerules, root);

		mpack_node_t worldgen = save_getMap(root, "worldgensettings");

		gWorld->worldInfo.seed	 = save_get(worldgen, u64, "seed", 0);
		gWorld->genSettings.type = save_get(worldgen, u8, "worldType", WorldGen_Default);

		mpack_error_t err = mpack_tree_destroy(&levelTree);
		if (err != mpack_ok) {
			Crash("Mpack error %d while loading world manifest\nPath: %s", err, gWorld->worldInfo.path);
		}
	}
}

#define PARAM_NUM_PER_PLAYER 14
void SaveManager_Unload(SaveManager* mgr) {
	chdir(gWorld->worldInfo.path);

	mpack_writer_t writer;
	mpack_writer_init_file(&writer, "level.mp");
	mpack_start_map(&writer, 4);

	save_cstr(&writer, "name", gWorld->worldInfo.name);

	// start player
	mpack_write_cstr(&writer, "players");
	mpack_start_array(&writer, 1);
	mpack_start_map(&writer, PARAM_NUM_PER_PLAYER);

	save_float(&writer, "x", gPlayer->position.x);
	save_float(&writer, "y", gPlayer->position.y);
	save_float(&writer, "z", gPlayer->position.z);

	save_float(&writer, "sx", gPlayer->spawnPos.x);
	save_float(&writer, "sy", gPlayer->spawnPos.y);
	save_float(&writer, "sz", gPlayer->spawnPos.z);

	save_float(&writer, "pitch", gPlayer->pitch);
	save_float(&writer, "yaw", gPlayer->yaw);

	save_u8(&writer, "hp", gPlayer->hp);
	save_u8(&writer, "hunger", gPlayer->hunger);
	save_u8(&writer, "gamemode", gPlayer->gamemode);

	save_bool(&writer, "cheats", gPlayer->cheats);
	save_bool(&writer, "flying", gPlayer->flying);
	save_bool(&writer, "crouching", gPlayer->crouching);

	mpack_finish_map(&writer);
	mpack_finish_array(&writer);
	// finish player

	// start gamerules
	mpack_write_cstr(&writer, "gamerules");
	mpack_start_map(&writer, GAMERULES_NUM);

	GameRules_Serialize(&gWorld->worldInfo.gamerules, &writer);

	mpack_finish_map(&writer);
	// finish gamerules

	// start worldgen
	mpack_write_cstr(&writer, "worldgensettings");
	mpack_start_map(&writer, 2);

	save_u64(&writer, "seed", gWorld->worldInfo.seed);
	save_u8(&writer, "worldType", gWorld->genSettings.type);

	mpack_finish_map(&writer);
	// finish worldgen

	mpack_finish_map(&writer);

	mpack_error_t err = mpack_writer_destroy(&writer);
	if (err != mpack_ok) {
		Crash("Mpack error %d while saving world manifest", err);
	}

	for (int i = 0; i < mgr->regions.length; i++) {
		Region_Deinit(mgr->regions.data[i]);
		free(mgr->regions.data[i]);
	}
	vec_clear(&mgr->regions);
}

static Region* fetchRegion(SaveManager* mgr, int x, int z) {
	if (gWorld) {
		for (int i = 0; i < mgr->regions.length; i++) {
			if (mgr->regions.data[i]->x == x && mgr->regions.data[i]->z == z) {
				return mgr->regions.data[i];
			}
		}
	}
	Region* region = (Region*)malloc(sizeof(Region));
	Region_Init(region, x, z);
	vec_push(&mgr->regions, region);
	svcSleepThread(50000);
	return region;
}

void SaveManager_LoadChunk(WorkerItem item, void* this) {
	SaveManager* mgr = (SaveManager*)this;
	int x			 = ChunkToRegionCoord(item.chunk->x);
	int z			 = ChunkToRegionCoord(item.chunk->z);
	Region* region	 = fetchRegion(mgr, x, z);

	Region_LoadChunk(region, item.chunk);
}
void SaveManager_SaveChunk(WorkerItem item, void* this) {
	SaveManager* mgr = (SaveManager*)this;
	int x			 = ChunkToRegionCoord(item.chunk->x);
	int z			 = ChunkToRegionCoord(item.chunk->z);

	Region* region = fetchRegion(mgr, x, z);

	Region_SaveChunk(region, item.chunk);
}
