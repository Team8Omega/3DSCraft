#include "world/GameRules.h"

#include "util/SerialUtils.h"
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
	serial_save_bool(writer, "doFireTick", gamerules->doFireTick);
	serial_save_bool(writer, "keepInventory", gamerules->keepInventory);
	serial_save_bool(writer, "naturalRegeneration", gamerules->naturalRegeneration);
	serial_save_bool(writer, "doDaylightCycle", gamerules->doDaylightCycle);
	serial_save_bool(writer, "showDeathMessages", gamerules->showDeathMessages);
	serial_save_int(writer, "randomTickSpeed", gamerules->randomTickSpeed);
	serial_save_bool(writer, "reducedDebugInfo", gamerules->reducedDebugInfo);
	serial_save_bool(writer, "showCoordinates", gamerules->showCoordinates);
	serial_save_int(writer, "spawnRadius", gamerules->spawnRadius);
	serial_save_bool(writer, "doWeatherCycle", gamerules->doWeatherCycle);
	serial_save_bool(writer, "announceAdvancements", gamerules->announceAdvancements);
	serial_save_bool(writer, "doImmediateRespawn", gamerules->doImmediateRespawn);
	serial_save_bool(writer, "drowningDamage", gamerules->drowningDamage);
	serial_save_bool(writer, "fallDamage", gamerules->fallDamage);
	serial_save_bool(writer, "fireDamage", gamerules->fireDamage);
	serial_save_bool(writer, "freezeDamage", gamerules->freezeDamage);
}
void GameRules_Deserialize(GameRules* gamerules, mpack_node_t root) {
	mpack_node_t node = serial_get_node(root, "gamerules");

	gamerules->doFireTick			= serial_get(node, bool, "doFireTick", gamerules->doFireTick);
	gamerules->keepInventory		= serial_get(node, bool, "keepInventory", gamerules->keepInventory);
	gamerules->naturalRegeneration	= serial_get(node, bool, "naturalRegeneration", gamerules->naturalRegeneration);
	gamerules->doDaylightCycle		= serial_get(node, bool, "doDaylightCycle", gamerules->doDaylightCycle);
	gamerules->showDeathMessages	= serial_get(node, bool, "showDeathMessages", gamerules->showDeathMessages);
	gamerules->randomTickSpeed		= serial_get(node, int, "randomTickSpeed", gamerules->randomTickSpeed);
	gamerules->reducedDebugInfo		= serial_get(node, bool, "reducedDebugInfo", gamerules->reducedDebugInfo);
	gamerules->showCoordinates		= serial_get(node, bool, "showCoordinates", gamerules->showCoordinates);
	gamerules->spawnRadius			= serial_get(node, int, "spawnRadius", gamerules->spawnRadius);
	gamerules->doWeatherCycle		= serial_get(node, bool, "doWeatherCycle", gamerules->doWeatherCycle);
	gamerules->announceAdvancements = serial_get(node, bool, "announceAdvancements", gamerules->announceAdvancements);
	gamerules->doImmediateRespawn	= serial_get(node, bool, "doImmediateRespawn", gamerules->doImmediateRespawn);
	gamerules->drowningDamage		= serial_get(node, bool, "drowningDamage", gamerules->drowningDamage);
	gamerules->fallDamage			= serial_get(node, bool, "fallDamage", gamerules->fallDamage);
	gamerules->fireDamage			= serial_get(node, bool, "fireDamage", gamerules->fireDamage);
	gamerules->freezeDamage			= serial_get(node, bool, "freezeDamage", gamerules->freezeDamage);
}