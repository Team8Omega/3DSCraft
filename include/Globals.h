#pragma once

#include "util/StringUtils.h"

#ifdef _DEBUG

#define DEBUG_BLOCKPLACE
#define DEBUG_INFO
#define DEBUG_LOG
#define DEBUG_UI

#endif

#define DEBUG_WORLDNAME

#define TEST_UNCRASH

// GET VERSION FROM MAKEFILE
#ifdef _DEBUG
#define VERSION_STRING_BAKER(major, minor, micro) "v" STRING_GET(major) "." STRING_GET(minor) "." STRING_GET(micro) " INDEV"
#else
#define VERSION_STRING_BAKER(major, minor, micro) "v" STRING_GET(major) "." STRING_GET(minor) "." STRING_GET(micro)
#endif
#define APP_VERSION VERSION_STRING_BAKER(_VER_MAJ, _VER_MIN, _VER_MIC)

#define AUTHOR STRING_GET(_AUTHOR)

extern bool gIsNew3ds;