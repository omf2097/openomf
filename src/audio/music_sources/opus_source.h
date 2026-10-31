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
/**
 * @brief Load an Opus file from a memory buffer and bind it to a music_source.
 * @param src Music source to load into
 * @param channels 1 for mono, 2 for stereo (must match the backend's output).
 * @param sample_rate Source sample rate
 * @param buf Buffer containing opus data
 * @param buflen Length of buffer
 * @return true on success.
 */

bool opus_load_memory(music_source *src, int channels, int sample_rate, const unsigned char *buffer, size_t buflen);

#endif // OPUS_SOURCE_H
