//
// Created by Elias on 27.07.2024.
//
#include "client/language/LanguageManager.h"
#include "lib/mpack/mpack.h"

void test(){
	mpack_tree_t tree;
	mpack_tree_init_filename(&tree, "homepage-example.mp", 0);
	mpack_tree_parse(&tree);
	mpack_node_t root = mpack_tree_root(&tree);

	// extract the example data on the msgpack homepage
	bool compact = mpack_node_bool(mpack_node_map_cstr(root, "compact"));
	int schema = mpack_node_i32(mpack_node_map_cstr(root, "schema"));

	// clean up and check for errors
	if (mpack_tree_destroy(&tree) != mpack_ok) {
		fprintf(stderr, "An error occurred decoding the data!\n");
		return;
	}
}