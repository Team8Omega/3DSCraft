#include "world/storage/SaveManager.h"

#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include "client/Crash.h"
#include "client/gui/DebugUI.h"
#include "util/Paths.h"
#include "util/SerialUtils.h"
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
		mpack_tree_t levelTree = serial_get_start("level.mp");
		mpack_node_t root	   = serial_get_root(&levelTree);

		serial_get_cstr(root, "name", gWorld->worldInfo.name);

		mpack_node_t players = serial_get_node(root, "players");
		mpack_node_t player	 = serial_get_mapArrayAt(players, 0);

		float x = serial_get(player, float, "x", 0);
		float y = serial_get(player, float, "y", 0) + 0.1f;
		float z = serial_get(player, float, "z", 0);
		Player_SetPosWorld(f3_new(x, y, z));

		gPlayer->spawnPos.x = serial_get(player, float, "sx", 0);
		gPlayer->spawnPos.y = serial_get(player, float, "sy", 0);
		gPlayer->spawnPos.z = serial_get(player, float, "sz", 0);

		gPlayer->pitch = serial_get(player, float, "pitch", 0.f);
		gPlayer->yaw   = serial_get(player, float, "yaw", 0.f);

		gPlayer->hp		  = serial_get(player, u8, "hp", 20);
		gPlayer->hunger	  = serial_get(player, u8, "hunger", 20);
		gPlayer->gamemode = serial_get(player, u8, "gamemode", Gamemode_Survival);

		gPlayer->flying	   = serial_get(player, bool, "flying", false);
		gPlayer->crouching = serial_get(player, bool, "crouching", false);
		gPlayer->cheats	   = serial_get(player, bool, "cheats", true);

		GameRules_Deserialize(&gWorld->worldInfo.gamerules, root);

		mpack_node_t worldgen = serial_get_node(root, "worldgensettings");

		gWorld->worldInfo.seed	 = serial_get(worldgen, u64, "seed", 0);
		gWorld->genSettings.type = serial_get(worldgen, u8, "worldType", WorldGen_Default);

		serial_get_end(&levelTree, "loading world manifest");
	}
}

#define PARAM_NUM_PER_PLAYER 14
void SaveManager_Unload(SaveManager* mgr) {
	chdir(gWorld->worldInfo.path);

	mpack_writer_t writer = serial_save_start("level.mp", 4);

	serial_save_cstr(&writer, "name", gWorld->worldInfo.name);

	// start player
	serial_save_array(&writer, "players", PARAM_NUM_PER_PLAYER);
	mpack_start_map(&writer, PARAM_NUM_PER_PLAYER);

	serial_save_float(&writer, "x", gPlayer->position.x);
	serial_save_float(&writer, "y", gPlayer->position.y);
	serial_save_float(&writer, "z", gPlayer->position.z);

	serial_save_float(&writer, "sx", gPlayer->spawnPos.x);
	serial_save_float(&writer, "sy", gPlayer->spawnPos.y);
	serial_save_float(&writer, "sz", gPlayer->spawnPos.z);

	serial_save_float(&writer, "pitch", gPlayer->pitch);
	serial_save_float(&writer, "yaw", gPlayer->yaw);

	serial_save_u8(&writer, "hp", gPlayer->hp);
	serial_save_u8(&writer, "hunger", gPlayer->hunger);
	serial_save_u8(&writer, "gamemode", gPlayer->gamemode);

	serial_save_bool(&writer, "cheats", gPlayer->cheats);
	serial_save_bool(&writer, "flying", gPlayer->flying);
	serial_save_bool(&writer, "crouching", gPlayer->crouching);

	serial_save_mapdone(&writer);
	mpack_finish_array(&writer);
	// finish player

	// start gamerules
	serial_save_map(&writer, "gamerules", GAMERULES_NUM);

	GameRules_Serialize(&gWorld->worldInfo.gamerules, &writer);

	serial_save_mapdone(&writer);
	// finish gamerules

	// start worldgen
	serial_save_map(&writer, "worldgensettings", 2);

	serial_save_u64(&writer, "seed", gWorld->worldInfo.seed);
	serial_save_u8(&writer, "worldType", gWorld->genSettings.type);

	serial_save_mapdone(&writer);
	// finish worldgen

	serial_save_mapdone(&writer);

	serial_save_end(&writer, "saving world manifest");

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
