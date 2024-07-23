#pragma once
#include <opus/opusfile.h>

typedef struct {
	const char *path;
	bool background;
	Thread threaid;
	OggOpusFile *opusFile;
} Sound;

const char *Sound_StrError(int error);

void Sound_Quit(int _channel);

bool Sound_Init(int _channel);

void Sound_Deinit(int _channel);

bool Sound_FillBuffer(int _channel, OggOpusFile *opusFile_, ndspWaveBuf *waveBuf_);

void Sound_AudioCallback(void *const nul_);

void Sound_AudioThread0(void *const opusFile_);

void Sound_AudioThread1(void *const opusFile_);

void Sound_PlayOpus(Sound *);