#pragma once

#include "client/model/CubeModel.h"

typedef struct {
	CubeModel* modelType;
	float3 position;
	float yaw;
	C3D_Tex* texture;
} Entity;

// WIP, this is to be replaced.
// TODO
void Entity_Draw(Entity* entity);
void Entity_Deinit(Entity* entity);
void Entity_SetTexture(Entity* entity, C3D_Tex* texture);