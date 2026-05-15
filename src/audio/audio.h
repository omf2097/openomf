/**
 * @file audio.h
 * @brief Public audio subsystem API
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

#include "audio/backends/audio_backend.h"
#include "audio/sound_opts.h"
#include "audio/sound_sources/sound_source.h"
#include "resources/ids.h"

/**
 * @brief Probe the built-in backends. Must be called once before audio_init.
 */
void audio_scan_backends(void);

/**
 * @brief Look up an available backend by index.
 * @param name Out: short name. May be NULL.
 * @param description Out: human-readable description. May be NULL.
 * @return true on success, false if index is out of range.
 */
bool audio_get_backend_info(int index, const char **name, const char **description);

/**
 * @brief Number of available backends.
 */
int audio_get_backend_count(void);

/**
 * @brief Initialize the audio subsystem.
 * @param try_name Backend name to use, or NULL/empty to auto-pick the first available.
 * @param resampler Music module resampler (backend-specific id; see psm_source / opus_source).
 * @param music_volume Initial music master volume (0.0..1.0).
 * @param sound_volume Initial sound master volume (0.0..1.0).
 * @return true on success.
 */
bool audio_init(const char *try_name, int sample_rate, bool mono, int resampler, float music_volume,
                float sound_volume);

/**
 * @brief Close the audio subsystem.
 */
void audio_close(void);

/**
 * @brief Play a stock SOUNDS.DAT sample by id. Convenience for callers without a source.
 * @return Backend handle for audio_fade_out, or -1 on failure.
 */
int audio_play_sound(int sound_id, const sound_opts *opts);

/**
 * @brief Play a caller-managed sound_source.
 * @details For callers that need to play a sub-range or a non-SOUNDS.DAT source —
 *          e.g. game_state's rollback replay that resumes at an offset.
 * @return Backend handle for audio_fade_out, or -1 on failure.
 */
int audio_play_source(const sound_source *src, const sound_opts *custom_opts);

/**
 * @brief Fade an in-flight playback to silence over `ms` milliseconds.
 * @param playback_id Handle returned by a previous audio_play_* call.
 */
void audio_fade_out(int playback_id, int ms);

/**
 * @brief Start background music playback (no-op if `id` is already current).
 */
void audio_play_music(resource_id id);

/**
 * @brief Stop music playback.
 */
void audio_stop_music(void);

/**
 * @brief Set music master volume (0.0..1.0).
 */
void audio_set_music_volume(float volume);

/**
 * @brief Set sound master volume (0.0..1.0).
 */
void audio_set_sound_volume(float volume);

/**
 * @brief Get the backend's supported sample-rate list.
 * @return Number of entries written to `*sample_rates`.
 */
unsigned audio_get_sample_rates(const audio_sample_rate **sample_rates);

/**
 * @brief Effective playback rate after applying `pitch` (matches the original game's sf tag).
 * @details Exposed for callers that need the resolved rate for their own bookkeeping
 *          (game_state's rollback offset math). audio_play_source applies it internally.
 */
int pitched_samplerate(int src_freq, int pitch);

#endif // AUDIO_H
