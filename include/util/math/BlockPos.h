#pragma once

typedef struct {
	int x;
	int y;
	int z;
} BlockPos;

static inline BlockPos Blockpos(int x, int y, int z) {
	return (BlockPos){ x, y, z };
}