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
#include <stdint.h>

#include "audio/backends/audio_backend.h"
#include "audio/music_sources/music_source.h"
#include "audio/sound_opts.h"
#include "audio/sound_sources/sound_source.h"

/**
 * @brief Value returned by the play functions when playback was not started
 */
#define AUDIO_INVALID_HANDLE UINT32_MAX

/**
 * @brief Probe the built-in backends. Must be called once before audio_init.
 */
void audio_scan_backends(void);

/**
 * @brief Look up an available backend by index.
 * @param index Backend index, 0 ... audio_get_backend_count() - 1.
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
 * @param sample_rate Output sample rate in Hz (see audio_get_sample_rates).
 * @param mono True for single-channel output, false for stereo.
 * @param resampler Music module resampler (backend-specific id; see psm_source / opus_source).
 * @param music_volume Initial music master volume (0.0 ... 1.0).
 * @param sound_volume Initial sound master volume (0.0 ... 1.0).
 * @return true on success.
 */
bool audio_init(const char *try_name, int sample_rate, bool mono, int resampler, float music_volume,
                float sound_volume);

/**
 * @brief Close the audio subsystem.
 */
void audio_close(void);

/**
 * @brief Play a UI/system SOUNDS.DAT sample. Volume is fixed at 64 (the original game's
 *        loud feedback level); panning is configurable so list-style widgets can sweep.
 * @param sound_id SOUNDS.DAT sample id.
 * @param panning Stereo panning, -100 ... 100 (0 = center).
 * @return Opaque playback handle, or AUDIO_INVALID_HANDLE on failure.
 */
uint32_t audio_play_sound_simple(int sound_id, int panning);

/**
 * @brief Play a caller-managed sound_source.
 * @details For callers that need to play a sub-range or a non-SOUNDS.DAT source --
 *          e.g. game_state's rollback replay that resumes at an offset.
 * @param src Pre-loaded sound source.
 * @param custom_opts Per-play parameters, or NULL for defaults.
 * @return Opaque playback handle, or AUDIO_INVALID_HANDLE on failure.
 */
uint32_t audio_play_source(const sound_source *src, const sound_opts *custom_opts);

/**
 * @brief Fade an in-flight playback to silence over `ms` milliseconds.
 * @param playback_id Handle returned by a previous audio_play_* call.
 * @param ms Fade-out duration in milliseconds.
 */
void audio_fade_out(uint32_t playback_id, int ms);

/**
 * @brief Update the stereo panning of an in-flight playback.
 * @param playback_id Handle returned by a previous audio_play_* call.
 * @param panning Stereo panning, -100 ... 100 (0 = center).
 */
void audio_set_pan(uint32_t playback_id, int panning);

/**
 * @brief Hand a music_source to the backend for playback.
 * @param src Preloaded music source.
 */
void audio_play_music(const music_source *src);

/**
 * @brief Stop music playback. Safe to call when nothing is playing.
 */
void audio_stop_music(void);

/**
 * @brief Read the backend's current output format.
 * @param sample_rate Out: output sample rate in Hz. May be NULL.
 * @param channels Out: output channel count (1 or 2). May be NULL.
 * @param resampler Out: backend-specific resampler id. May be NULL.
 */
void audio_get_music_info(unsigned *sample_rate, unsigned *channels, unsigned *resampler);

/**
 * @brief Set music master volume.
 * @param volume Volume level, 0.0 ... 1.0.
 */
void audio_set_music_volume(float volume);

/**
 * @brief Set sound master volume.
 * @param volume Volume level, 0.0 ... 1.0.
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
 * @param src_freq Source's native sample rate in Hz.
 * @param pitch Pitch adjustment.
 * @return Effective sample rate in Hz.
 */
int pitched_samplerate(int src_freq, int pitch);

#endif // AUDIO_H
