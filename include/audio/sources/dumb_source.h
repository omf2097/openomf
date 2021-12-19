#ifndef DUMB_SOURCE_H
#define DUMB_SOURCE_H

#ifdef USE_DUMB

#include "audio/source.h"

int dumb_source_init(audio_source *src, const char* file, int channels, int freq, int resampler);
audio_source_freq* dumb_get_freqs();
audio_source_resampler* dumb_get_resamplers();

#endif // USE_DUMB

#endif // DUMB_SOURCE_H
