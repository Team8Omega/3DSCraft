#pragma once

#include "util/StringUtils.h"

typedef enum
{
	GameState_Menu,
	GameState_Playing,
	GameState_Paused,
} GameState;

#ifdef _DEBUG
#pragma message "WARNING: Compiling with Debug-Flag"

#define DEBUG_BLOCKPLACE 1
#endif

// GET VERSION FROM MAKEFILE
#ifdef _DEBUG
#define VERSION_STRING_BAKER(major, minor, micro) "v" STRING_GET(major) "." STRING_GET(minor) "." STRING_GET(micro) " INDEV"
#else
#define VERSION_STRING_BAKER(major, minor, micro) "v" STRING_GET(major) "." STRING_GET(minor) "." STRING_GET(micro)
#endif
#define APP_VERSION VERSION_STRING_BAKER(_VER_MAJ, _VER_MIN, _VER_MIC)

#define AUTHOR "Team Omega"
