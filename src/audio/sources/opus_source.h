#ifndef OPUS_SOURCE_H
#define OPUS_SOURCE_H

#include <stdbool.h>

#include "audio/sources/music_source.h"

bool opus_load(music_source *src, int channels, int sample_rate, const char *file);
bool opus_load_memory(music_source *src, int channels, int sample_rate, const unsigned char *buffer, size_t buflen);

#endif // OPUS_SOURCE_H
