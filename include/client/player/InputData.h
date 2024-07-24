#pragma once

#include <stdint.h>

typedef struct {
	u32 keysheld, keysdown, keysup;
	s16 circlePadX, circlePadY;
	u16 touchX, touchY;
	s16 cStickX, cStickY;
} InputData;

extern InputData gInput;
extern InputData gInputOld;