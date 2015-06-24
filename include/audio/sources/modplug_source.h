#ifndef _MODPLUG_SOURCE
#define _MODPLUG_SOURCE

#ifdef USE_MODPLUG

#include "audio/source.h"

int modplug_source_init(audio_source *src, const char* file, int channels, int freq, int resampler);
audio_source_freq* modplug_get_freqs();
audio_source_resampler* modplug_get_resamplers();

#endif // USE_MODPLUG

#endif // _MODPLUG_SOURCE
