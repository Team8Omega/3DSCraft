#pragma once

typedef u8 DamageCause;
enum
{
	DAMAGECAUSE_NONE,
	DAMAGECAUSE_FIRE,
	DAMAGECAUSE_POISON,
	DAMAGECAUSE_COUNT
};

typedef struct {
	DamageCause cause;
	int time;
} Damage;
