#pragma once

#include <mpack/mpack.h>
#include <stdio.h>
#include <vec/vec.h>

#include "client/Crash.h"
#include "client/player/Player.h"
#include "world/storage/Region.h"

typedef struct {
	vec_t(Region*) regions;
} SaveManager;

void SaveManager_InitFileSystem();

void SaveManager_Init(SaveManager* mgr);
void SaveManager_Deinit(SaveManager* mgr);

void SaveManager_Load(SaveManager* mgr);
void SaveManager_Unload(SaveManager* mgr);

void SaveManager_LoadChunk(WorkerItem item, void* this);
void SaveManager_SaveChunk(WorkerItem item, void* this);

static void inline save_cstr(mpack_writer_t* writer, const char* cstr, const char* v) {
	mpack_write_cstr(writer, cstr);
	mpack_write_cstr(writer, v);
}
static void inline save_bool(mpack_writer_t* writer, const char* cstr, bool v) {
	mpack_write_cstr(writer, cstr);
	mpack_save_bool(writer, v);
}
static void inline save_int(mpack_writer_t* writer, const char* cstr, int v) {
	mpack_write_cstr(writer, cstr);
	mpack_save_int(writer, v);
}
static void inline save_u8(mpack_writer_t* writer, const char* cstr, u8 v) {
	mpack_write_cstr(writer, cstr);
	mpack_save_u8(writer, v);
}
static void inline save_u64(mpack_writer_t* writer, const char* cstr, u64 v) {
	mpack_write_cstr(writer, cstr);
	mpack_write_u64(writer, v);
}
static void inline save_float(mpack_writer_t* writer, const char* cstr, float v) {
	mpack_write_cstr(writer, cstr);
	mpack_save_float(writer, v);
}

static int inline testing(const char* name) {
	Crash("Not found: %s", name);
	return 1;
}

#define save_getMap(node, name) mpack_node_map_cstr(root, name);
#define save_getMapArray(node, name, index) mpack_node_array_at(mpack_node_map_cstr(node, name), index);
#define save_get(node, typ, key, default_)                                                                                                 \
	((mpack_node_type(mpack_node_map_cstr_optional((node), (key))) != mpack_type_nil)                                                      \
		 ? mpack_node_##typ(mpack_node_map_cstr_optional((node), (key)))                                                                   \
		 : testing(key))
