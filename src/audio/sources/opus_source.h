#ifndef OPUS_SOURCE_H
#define OPUS_SOURCE_H

#include <stdbool.h>

#include "audio/sources/music_source.h"

bool opus_load(music_source *src, int channels, int sample_rate, const char *file);

#endif // OPUS_SOURCE_H
