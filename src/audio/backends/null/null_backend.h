/**
 * @file null_backend.h
 * @brief No-op audio backend for headless / test builds
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef NULL_BACKEND_H
#define NULL_BACKEND_H

#include "audio/backends/audio_backend.h"

/**
 * @brief Populate `null_backend` with the no-op backend's callbacks.
 * @param null_backend Backend vtable to fill.
 */
void null_audio_backend_set_callbacks(audio_backend *null_backend);

/**
 * @brief Clear all per-channel spy state. Call between tests.
 */
void null_audio_backend_reset_state(void);

/**
 * @brief Spy: how many times play_pcm_sound succeeded on this channel.
 * @param channel Channel index.
 * @return Cumulative successful play count.
 */
int null_audio_backend_get_play_count(int channel);

/**
 * @brief Spy: how many times fade_out_channel was called on this channel.
 * @param channel Channel index.
 * @return Cumulative fade-out call count.
 */
int null_audio_backend_get_fade_count(int channel);

/**
 * @brief Spy: how many times stop_channel was called on this channel.
 * @param channel Channel index.
 * @return Cumulative stop call count.
 */
int null_audio_backend_get_stop_count(int channel);

/**
 * @brief Spy: sound_id of the most recent play on this channel.
 * @param channel Channel index.
 * @return Most recent sound_id, or -1 if nothing has played on this channel.
 */
int null_audio_backend_get_last_sound_id(int channel);

/**
 * @brief Spy: most recent pan value handed to the backend for this channel.
 * @param channel Channel index, 0 ..< SOUND_CHANNEL_COUNT.
 * @return Last pan value (from play or set_channel_panning), 0 if nothing yet.
 */
int null_audio_backend_get_last_pan(int channel);

/**
 * @brief Spy: how many times set_channel_panning was called on this channel.
 * @param channel Channel index, 0 ..< SOUND_CHANNEL_COUNT.
 * @return Cumulative set_channel_panning call count.
 */
int null_audio_backend_get_pan_update_count(int channel);

#endif // NULL_BACKEND_H
