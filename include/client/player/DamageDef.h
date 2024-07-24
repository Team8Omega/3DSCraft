#pragma once

typedef enum
{
	DAMAGECAUSE_NONE,
	DAMAGECAUSE_FIRE,
	DAMAGECAUSE_POISON,
	DAMAGECAUSE_COUNT
} DamageCause;

typedef struct {
	DamageCause cause;
	int time;
} Damage;
