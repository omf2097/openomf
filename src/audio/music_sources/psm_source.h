/**
 * @file psm_source.h
 * @brief PSM (MASI module) music source via libxmp
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef PSM_SOURCE_H
#define PSM_SOURCE_H

#include <stdbool.h>

#include "audio/music_sources/music_source.h"

/**
 * @brief Load a PSM file and bind it to a music_source.
 * @param src Music source to load into
 * @param channels 1 for mono, 2 for stereo (must match the backend's output).
 * @param sample_rate Source sample rate
 * @param resampler libxmp resampler id (see psm_get_resamplers).
 * @param file File path to load
 * @return true on success.
 */
bool psm_load(music_source *src, int channels, int sample_rate, int resampler, const char *file);

/**
 * @brief Resampler options recognized by psm_load. Returns number of entries.
 */
unsigned psm_get_resamplers(const music_resampler **resamplers);

#endif // PSM_SOURCE_H
