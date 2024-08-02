#include "entity/Entity.h"

#include "client/Crash.h"

void Entity_Draw(Entity* entity) {
	C3D_Mtx movement;
	Mtx_Identity(&movement);
	Mtx_Translate(&movement, entity->position.x, entity->position.y, entity->position.z, true);
	Mtx_RotateY(&movement, entity->yaw, true);

	C3D_TexBind(0, entity->texture);

	CubeModel_Draw(entity->modelType, &movement);
}
void Entity_Deinit(Entity* entity) {
	linearFree(entity);
}
void Entity_SetTexture(Entity* entity, C3D_Tex* texture) {
	entity->texture = texture;
}