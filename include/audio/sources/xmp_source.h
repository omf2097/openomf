#ifndef XMP_SOURCE_H
#define XMP_SOURCE_H

#ifdef USE_XMP

#include "audio/source.h"

int xmp_source_init(audio_source *src, const char* file, int channels, int freq, int resampler);
audio_source_freq* xmp_get_freqs();
audio_source_resampler* xmp_get_resamplers();

#endif // USE_XMP

#endif // XMP_SOURCE_H
