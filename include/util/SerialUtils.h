#pragma once

#include <mpack/mpack.h>

static mpack_writer_t inline serial_save_start(const char* filename, u8 numKeys) {
	mpack_writer_t writer;
	mpack_writer_init_file(&writer, filename);
	mpack_start_map(&writer, numKeys);
}
static bool inline serial_save_end(mpack_writer_t* writer, const char* context) {
	mpack_error_t err = mpack_writer_destroy(&writer);
	if (err != mpack_ok) {
		Crash("Mpack error %d while %s", err, context);
		return false;
	}
	return true;
}
static mpack_node_t serial_get_root(mpack_tree_t* tree) {
	return mpack_tree_root(tree);
}
static void inline serial_save_cstr(mpack_writer_t* writer, const char* cstr, const char* v) {
	mpack_write_cstr(writer, cstr);
	mpack_write_cstr(writer, v);
}
static void inline serial_save_bool(mpack_writer_t* writer, const char* cstr, bool v) {
	mpack_write_cstr(writer, cstr);
	mpack_save_bool(writer, v);
}
static void inline serial_save_int(mpack_writer_t* writer, const char* cstr, int v) {
	mpack_write_cstr(writer, cstr);
	mpack_save_int(writer, v);
}
static void inline serial_save_u8(mpack_writer_t* writer, const char* cstr, u8 v) {
	mpack_write_cstr(writer, cstr);
	mpack_save_u8(writer, v);
}
static void inline serial_save_u64(mpack_writer_t* writer, const char* cstr, u64 v) {
	mpack_write_cstr(writer, cstr);
	mpack_write_u64(writer, v);
}
static void inline serial_save_float(mpack_writer_t* writer, const char* cstr, float v) {
	mpack_write_cstr(writer, cstr);
	mpack_save_float(writer, v);
}
static void inline serial_save_map(mpack_writer_t* writer, const char* cstr, u8 num) {
	mpack_write_cstr(&writer, cstr);
	mpack_start_map(&writer, num);
}
static void inline serial_save_mapdone(mpack_writer_t* writer) {
	mpack_finish_map(&writer);
}
static void inline serial_save_array(mpack_writer_t* writer, const char* cstr, u8 num) {
	mpack_write_cstr(&writer, cstr);
	mpack_start_array(&writer, num);
}
static void inline serial_save_arraydone(mpack_writer_t* writer) {
	mpack_finish_array(&writer);
}

#define serial_get_node(node, name) mpack_node_map_cstr(node, name);
#define serial_get_arrayAt(node, typ, idx) mpack_node_##typ(mpack_node_map_int_optional((node), (idx)))
#define serial_get_mapArrayAt(node, index) mpack_node_array_at(node, index);
#define serial_is(node, key, type) mpack_node_type(mpack_node_map_cstr_optional((node), (key))) == mpack_type_##type
#define serial_has(node, key) serial_is(node, key, nil)
#define serial_get(node, typ, key, default_)                                                                                               \
	(serial_has(node, key) ? mpack_node_##typ(mpack_node_map_cstr_optional((node), (key))) : (default_))

static mpack_tree_t serial_get_start(const char* filename) {
	mpack_tree_t levelTree;
	mpack_tree_init_file(&levelTree, filename, 0);
}
static bool serial_get_end(mpack_tree_t* tree, const char* context) {
	mpack_error_t err = mpack_tree_destroy(&tree);
	if (err != mpack_ok) {
		Crash("Mpack error %d while %s", context);
		return false;
	}
	return true;
}
static void serial_get_cstr(mpack_node_t root, const char* keyName, char* destination) {
	mpack_node_copy_utf8_cstr(mpack_node_map_cstr(root, keyName), destination, sizeof(destination));
}
static inline size_t serial_get_arrayLength(mpack_node_t node) {
	mpack_node_array_length(node);
}