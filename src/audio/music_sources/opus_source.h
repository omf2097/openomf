/**
 * @file opus_source.h
 * @brief Opus music source via opusfile (with LOOPSTART/LOOPEND tag support)
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef OPUS_SOURCE_H
#define OPUS_SOURCE_H

#include <stdbool.h>

#include "audio/music_sources/music_source.h"

/**
 * @brief Load an Opus file and bind it to a music_source.
 * @param src Music source to load into
 * @param channels 1 for mono, 2 for stereo (must match the backend's output).
 * @param sample_rate Source sample rate
 * @param file File path to load
 * @return true on success.
 */
bool opus_load(music_source *src, int channels, int sample_rate, const char *file);

#endif // OPUS_SOURCE_H
