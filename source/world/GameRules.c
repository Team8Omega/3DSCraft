#include "world/GameRules.h"

#include "world/storage/SaveManager.h"

void GameRules_Reset(GameRules* gamerules) {
	gamerules->doFireTick			= true;
	gamerules->keepInventory		= false;
	gamerules->naturalRegeneration	= true;
	gamerules->doDaylightCycle		= true;
	gamerules->showDeathMessages	= true;
	gamerules->randomTickSpeed		= 3;
	gamerules->reducedDebugInfo		= false;
	gamerules->showCoordinates		= false;
	gamerules->spawnRadius			= 10;
	gamerules->doWeatherCycle		= true;
	gamerules->announceAdvancements = true;
	gamerules->doImmediateRespawn	= false;
	gamerules->drowningDamage		= true;
	gamerules->fallDamage			= true;
	gamerules->fireDamage			= true;
	gamerules->freezeDamage			= true;
}
void GameRules_Serialize(GameRules* gamerules, mpack_writer_t* writer) {
	save_bool(writer, "doFireTick", gamerules->doFireTick);
	save_bool(writer, "keepInventory", gamerules->keepInventory);
	save_bool(writer, "naturalRegeneration", gamerules->naturalRegeneration);
	save_bool(writer, "doDaylightCycle", gamerules->doDaylightCycle);
	save_bool(writer, "showDeathMessages", gamerules->showDeathMessages);
	save_int(writer, "randomTickSpeed", gamerules->randomTickSpeed);
	save_bool(writer, "reducedDebugInfo", gamerules->reducedDebugInfo);
	save_bool(writer, "showCoordinates", gamerules->showCoordinates);
	save_int(writer, "spawnRadius", gamerules->spawnRadius);
	save_bool(writer, "doWeatherCycle", gamerules->doWeatherCycle);
	save_bool(writer, "announceAdvancements", gamerules->announceAdvancements);
	save_bool(writer, "doImmediateRespawn", gamerules->doImmediateRespawn);
	save_bool(writer, "drowningDamage", gamerules->drowningDamage);
	save_bool(writer, "fallDamage", gamerules->fallDamage);
	save_bool(writer, "fireDamage", gamerules->fireDamage);
	save_bool(writer, "freezeDamage", gamerules->freezeDamage);
}
void GameRules_Deserialize(GameRules* gamerules, mpack_node_t root) {
	mpack_node_t node = save_getMap(root, "gamerules");

	gamerules->doFireTick			= save_get(node, bool, "doFireTick", gamerules->doFireTick);
	gamerules->keepInventory		= save_get(node, bool, "keepInventory", gamerules->keepInventory);
	gamerules->naturalRegeneration	= save_get(node, bool, "naturalRegeneration", gamerules->naturalRegeneration);
	gamerules->doDaylightCycle		= save_get(node, bool, "doDaylightCycle", gamerules->doDaylightCycle);
	gamerules->showDeathMessages	= save_get(node, bool, "showDeathMessages", gamerules->showDeathMessages);
	gamerules->randomTickSpeed		= save_get(node, int, "randomTickSpeed", gamerules->randomTickSpeed);
	gamerules->reducedDebugInfo		= save_get(node, bool, "reducedDebugInfo", gamerules->reducedDebugInfo);
	gamerules->showCoordinates		= save_get(node, bool, "showCoordinates", gamerules->showCoordinates);
	gamerules->spawnRadius			= save_get(node, int, "spawnRadius", gamerules->spawnRadius);
	gamerules->doWeatherCycle		= save_get(node, bool, "doWeatherCycle", gamerules->doWeatherCycle);
	gamerules->announceAdvancements = save_get(node, bool, "announceAdvancements", gamerules->announceAdvancements);
	gamerules->doImmediateRespawn	= save_get(node, bool, "doImmediateRespawn", gamerules->doImmediateRespawn);
	gamerules->drowningDamage		= save_get(node, bool, "drowningDamage", gamerules->drowningDamage);
	gamerules->fallDamage			= save_get(node, bool, "fallDamage", gamerules->fallDamage);
	gamerules->fireDamage			= save_get(node, bool, "fireDamage", gamerules->fireDamage);
	gamerules->freezeDamage			= save_get(node, bool, "freezeDamage", gamerules->freezeDamage);
}