/**
 * @file audio_sources.h
 * @brief Source selection for sound and music playback
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 * @details Decides which loader runs for a given sound id.
 */

#ifndef AUDIO_SOURCES_H
#define AUDIO_SOURCES_H

#include "audio/music_sources/music_source.h"
#include "audio/sound_sources/sound_source.h"
#include "resources/ids.h"

#include <stdbool.h>

/**
 * @brief Source-selection mode.
 */
typedef enum
{
    AUDIO_SOURCES_REAL = 0, ///< Load from SOUNDS.DAT / PSM / OGG (default).
    AUDIO_SOURCES_NULL = 1, ///< NULL sources (Debug builds only).
} audio_sources_mode;

/**
 * @brief Set the active source mode. Affects both sound and music pickers.
 * @param mode New mode.
 */
void audio_sources_set_mode(audio_sources_mode mode);

/**
 * @brief Get the active source mode.
 * @return Current mode.
 */
audio_sources_mode audio_sources_get_mode(void);

/**
 * @brief Load a sound source for id using the current mode.
 * @param src Sound source to populate.
 * @param sound_id Sample id.
 * @return true on success, false if the id is invalid or the slot is empty.
 */
bool sound_source_pick(sound_source *src, int sound_id);

/**
 * @brief Load a music source for id using the current mode.
 * @param src Music source to populate.
 * @param id Music resource id.
 * @param channels Backend channel count (1=mono, 2=stereo).
 * @param sample_rate Backend output sample rate.
 * @param resampler Backend-specific resampler id (used only for PSM).
 * @return true on success.
 */
bool music_source_pick(music_source *src, resource_id id, unsigned channels, unsigned sample_rate, unsigned resampler);

#endif // AUDIO_SOURCES_H
