#pragma once

#include <vec/vec.h>

#include "world/chunk/Chunk.h"

#include "util/math/Xorshift.h"

#include <stdbool.h>

#include <3ds.h>

typedef u8 WorkerItemType;
enum
{
	WorkerItemType_Load,
	WorkerItemType_Save,
	WorkerItemType_BaseGen,
	WorkerItemType_Decorate,
	WorkerItemType_PolyGen,
	WorkerItemTypes_Count
};

typedef struct {
	WorkerItemType type;
	Chunk* chunk;
	u32 uuid;
} WorkerItem;

typedef struct {
	vec_t(WorkerItem) queue;

	LightEvent itemAddedEvent;
	LightLock listInUse;
} WorkQueue;

extern WorkQueue gWorkqueue;

static inline void WorkQueue_Init() {
	vec_init(&gWorkqueue.queue);
	LightLock_Init(&gWorkqueue.listInUse);
	LightEvent_Init(&gWorkqueue.itemAddedEvent, RESET_STICKY);
}
static inline void WorkQueue_Deinit() {
	vec_deinit(&gWorkqueue.queue);
}

static inline void WorkQueue_AddItem(WorkerItem item) {
	item.uuid = item.chunk->uuid;
	++item.chunk->tasksRunning;
	if (item.type == WorkerItemType_PolyGen)
		++item.chunk->graphicalTasksRunning;
	LightLock_Lock(&gWorkqueue.listInUse);
	vec_push(&gWorkqueue.queue, item);
	LightLock_Unlock(&gWorkqueue.listInUse);

	LightEvent_Signal(&gWorkqueue.itemAddedEvent);
}