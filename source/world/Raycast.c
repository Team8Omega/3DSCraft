
#include "world/Raycast.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <citro3d.h>

#define INF (CHUNKCACHE_SIZE / 2 * CHUNK_SIZE)

bool Raycast_Cast(float3 inpos, float3 raydir, Raycast_Result* out) {
	int mapX = FastFloor(inpos.x), mapY = FastFloor(inpos.y), mapZ = FastFloor(inpos.z);

	float xSqr = raydir.x * raydir.x;
	float ySqr = raydir.y * raydir.y;
	float zSqr = raydir.z * raydir.z;

	float deltaDistX = sqrtf(1.f + (ySqr + zSqr) / xSqr);
	float deltaDistY = sqrtf(1.f + (xSqr + zSqr) / ySqr);
	float deltaDistZ = sqrtf(1.f + (xSqr + ySqr) / zSqr);

	int stepX, stepY, stepZ;
	float sideDistX, sideDistY, sideDistZ;
	if (raydir.x < 0) {
		stepX	  = -1;
		sideDistX = (inpos.x - mapX) * deltaDistX;
	} else {
		stepX	  = 1;
		sideDistX = (mapX + 1.f - inpos.x) * deltaDistX;
	}
	if (raydir.y < 0) {
		stepY	  = -1;
		sideDistY = (inpos.y - mapY) * deltaDistY;
	} else {
		stepY	  = 1;
		sideDistY = (mapY + 1.f - inpos.y) * deltaDistY;
	}
	if (raydir.z < 0) {
		stepZ	  = -1;
		sideDistZ = (inpos.z - mapZ) * deltaDistZ;
	} else {
		stepZ	  = 1;
		sideDistZ = (mapZ + 1.f - inpos.z) * deltaDistZ;
	}

	bool isHit = false;
	Axis side  = Axis_Z;
	u8 steps   = 0;
	while (!isHit) {
		if (sideDistX < sideDistY && sideDistX < sideDistZ) {
			sideDistX += deltaDistX;
			mapX += stepX;
			side = Axis_X;
		} else if (sideDistY < sideDistZ) {
			sideDistY += deltaDistY;
			mapY += stepY;
			side = Axis_Y;
		} else {
			sideDistZ += deltaDistZ;
			mapZ += stepZ;
			side = Axis_Z;
		}
		if (World_GetBlock(mapX, mapY, mapZ) != BLOCK_AIR /* || World_GetBlock(mapX, mapY, mapZ) == Block_Lava*/)
			isHit = true;

		if (steps++ > INF)
			break;
	}

	switch (side) {
		case Axis_X:
			if (raydir.x > 0.f)
				out->direction = Direction_West;
			else
				out->direction = Direction_East;
			break;
		case Axis_Y:
			if (raydir.y > 0.f)
				out->direction = Direction_Bottom;
			else
				out->direction = Direction_Top;
			break;
		case Axis_Z:
			if (raydir.z > 0.f)
				out->direction = Direction_North;
			else
				out->direction = Direction_South;
			break;
	}

	float3 dist	 = f3_sub(f3_new(mapX, mapY, mapZ), inpos);
	out->distSqr = f3_magSqr(dist);
	out->x		 = mapX;
	out->y		 = mapY;
	out->z		 = mapZ;

	return isHit;
}
