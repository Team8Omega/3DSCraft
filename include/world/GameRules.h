#pragma once

#include <3ds.h>

typedef struct {
	bool doFireTick;
	bool keepInventory;
	bool naturalRegeneration;
	bool doDaylightCycle;
	bool showDeathMessages;
	int randomTickSpeed;
	bool reducedDebugInfo;
	bool showCoordinates;
	int spawnRadius;
	bool doWeatherCycle;
	bool announceAdvancements;
	bool doImmediateRespawn;
	bool drowningDamage;
	bool fallDamage;
	bool fireDamage;
	bool freezeDamage;
} GameRules;

#define GAMERULES_NUM 16

typedef struct mpack_writer_t mpack_writer_t;
typedef struct mpack_node_t mpack_node_t;

void GameRules_Reset(GameRules* gameRules);
void GameRules_Serialize(GameRules* gamerules, mpack_writer_t* writer);
void GameRules_Deserialize(GameRules* gamerules, mpack_node_t root);