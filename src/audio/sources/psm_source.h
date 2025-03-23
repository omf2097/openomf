#ifndef XMP_SOURCE_H
#define XMP_SOURCE_H

#include <stdbool.h>

#include "audio/sources/music_source.h"

bool psm_load(music_source *src, int channels, int sample_rate, int resampler, const char *file);
unsigned psm_get_resamplers(const music_resampler **resamplers);

#endif // XMP_SOURCE_H
