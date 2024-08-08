#include "world/level/storage/SaveManager.h"

#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include <mpack/mpack.h>

#include "client/Crash.h"
#include "client/gui/DebugUI.h"
#include "util/Paths.h"

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

#define mpack_get(node, typ, key, default_)                                                                                                \
	((mpack_node_type(mpack_node_map_cstr_optional((node), (key))) != mpack_type_nil)                                                      \
		 ? mpack_node_##typ(mpack_node_map_cstr_optional((node), (key)))                                                                   \
		 : (default_))

void SaveManager_Load(SaveManager* mgr) {
	// char buffer[256];
	// sprintf(buffer, "%s", gWorld.path);

	mkdir(gWorld.path, mkdirFlags);
	if (access(gWorld.path, F_OK))
		Crash("World path invalid, internal error. Cannot create world folder.\n\nPath: %s", gWorld.path);

	chdir(gWorld.path);

	mkdir("regions", mkdirFlags);

	// printf(buffer, "%s/level.mp", gWorld.path);

	if (access("level.mp", F_OK) != -1) {
		mpack_tree_t levelTree;
		mpack_tree_init_file(&levelTree, "level.mp", 0);
		mpack_node_t root = mpack_tree_root(&levelTree);

		mpack_node_copy_utf8_cstr(mpack_node_map_cstr(root, "name"), gWorld.name, sizeof(gWorld.name));

		gWorld.genSettings.type = mpack_get(root, u8, "worldType", WorldGen_Default);

		mpack_node_t player = mpack_node_array_at(mpack_node_map_cstr(root, "players"), 0);

		gPlayer.position.x = mpack_get(player, float, "x", 0);
		gPlayer.position.y = mpack_get(player, float, "y", 0) + 0.1f;
		gPlayer.position.z = mpack_get(player, float, "z", 0);

		gPlayer.spawnPos.x = mpack_get(player, float, "sx", 0);
		gPlayer.spawnPos.y = mpack_get(player, float, "sy", 0);
		gPlayer.spawnPos.z = mpack_get(player, float, "sz", 0);

		gPlayer.pitch = mpack_get(player, float, "pitch", 0.f);
		gPlayer.yaw	  = mpack_get(player, float, "yaw", 0.f);

		gPlayer.hp		 = mpack_get(player, u8, "hp", 20);
		gPlayer.hunger	 = mpack_get(player, u8, "hunger", 20);
		gPlayer.gamemode = mpack_get(player, u8, "gamemode", Gamemode_Survival);

		gPlayer.flying	  = mpack_get(player, bool, "flying", false);
		gPlayer.crouching = mpack_get(player, bool, "crouching", false);
		gPlayer.cheats	  = mpack_get(player, bool, "cheats", true);

		mpack_error_t err = mpack_tree_destroy(&levelTree);
		if (err != mpack_ok) {
			Crash("Mpack error %d while loading world manifest\nPath: %s", err, gWorld.path);
		}
	}
}

static void inline write_str(mpack_writer_t* writer, const char* cstr, const char* v) {
	mpack_write_cstr(writer, cstr);
	mpack_write_cstr(writer, v);
}
static void inline write_bool(mpack_writer_t* writer, const char* cstr, bool v) {
	mpack_write_cstr(writer, cstr);
	mpack_write_bool(writer, v);
}
static void inline write_int(mpack_writer_t* writer, const char* cstr, int v) {
	mpack_write_cstr(writer, cstr);
	mpack_write_int(writer, v);
}
static void inline write_u8(mpack_writer_t* writer, const char* cstr, u8 v) {
	mpack_write_cstr(writer, cstr);
	mpack_write_u8(writer, v);
}
static void inline write_float(mpack_writer_t* writer, const char* cstr, float v) {
	mpack_write_cstr(writer, cstr);
	mpack_write_float(writer, v);
}

#define PARAM_NUM_PER_PLAYER 14
void SaveManager_Unload(SaveManager* mgr) {
	mpack_writer_t writer;
	mpack_writer_init_file(&writer, "level.mp");
	mpack_start_map(&writer, 3);

	write_str(&writer, "name", gWorld.name);

	write_u8(&writer, "worldType", gWorld.genSettings.type);

	mpack_write_cstr(&writer, "players");
	mpack_start_array(&writer, 1);
	mpack_start_map(&writer, PARAM_NUM_PER_PLAYER);

	write_float(&writer, "x", gPlayer.position.x);
	write_float(&writer, "y", gPlayer.position.y);
	write_float(&writer, "z", gPlayer.position.z);

	write_float(&writer, "sx", gPlayer.spawnPos.x);
	write_float(&writer, "sy", gPlayer.spawnPos.y);
	write_float(&writer, "sz", gPlayer.spawnPos.z);

	write_float(&writer, "pitch", gPlayer.pitch);
	write_float(&writer, "yaw", gPlayer.yaw);

	write_u8(&writer, "hp", gPlayer.hp);
	write_u8(&writer, "hunger", gPlayer.hunger);
	write_u8(&writer, "gamemode", gPlayer.gamemode);

	write_bool(&writer, "cheats", gPlayer.cheats);
	write_bool(&writer, "flying", gPlayer.flying);
	write_bool(&writer, "crouching", gPlayer.crouching);

	mpack_finish_map(&writer);
	mpack_finish_array(&writer);

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
	if (gWorld.active) {
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

void SaveManager_LoadChunk(WorkQueue* queue, WorkerItem item, void* this) {
	SaveManager* mgr = (SaveManager*)this;
	int x			 = ChunkToRegionCoord(item.chunk->x);
	int z			 = ChunkToRegionCoord(item.chunk->z);
	Region* region	 = fetchRegion(mgr, x, z);

	Region_LoadChunk(region, item.chunk);
}
void SaveManager_SaveChunk(WorkQueue* queue, WorkerItem item, void* this) {
	SaveManager* mgr = (SaveManager*)this;
	int x			 = ChunkToRegionCoord(item.chunk->x);
	int z			 = ChunkToRegionCoord(item.chunk->z);

	Region* region = fetchRegion(mgr, x, z);

	Region_SaveChunk(region, item.chunk);
}
