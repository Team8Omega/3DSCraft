#pragma once

typedef struct {
	char* cause;
	int time;
} Damage;

void OvertimeDamage(const char* cause, int time);
