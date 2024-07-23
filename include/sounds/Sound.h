#pragma once
#include <opus/opusfile.h>

typedef struct {
	const char *path;
	bool background;
	Thread *threaid;
	OggOpusFile *opusFile;
} Sound;

void playopus(Sound *);
