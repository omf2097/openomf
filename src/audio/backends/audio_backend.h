/**
 * @file audio_backend.h
 * @brief Audio backend vtable
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef AUDIO_BACKEND_H
#define AUDIO_BACKEND_H

#include "audio/music_sources/music_source.h"
#include "audio/sound_sources/sound_source.h"

#include <stdbool.h>
#include <stddef.h>

#define SOUND_CHANNEL_COUNT 3

/**
 * @brief One entry in a backend's supported sample-rate list.
 */
typedef struct audio_sample_rate {
    unsigned int sample_rate;
    bool is_default;
    const char *name;
} audio_sample_rate;

typedef struct audio_backend audio_backend;

typedef bool (*is_backend_available_fn)(void);
typedef const char *(*get_backend_description_fn)(void);
typedef const char *(*get_backend_name_fn)(void);
typedef unsigned int (*get_backend_sample_rates_fn)(const audio_sample_rate **sample_rates);

/// Read current context settings (any out-param may be NULL).
typedef void (*get_backend_info_fn)(void *ctx, unsigned *sample_rate, unsigned *channels, unsigned *resampler);

// Lifecycle.
typedef void (*create_backend_fn)(audio_backend *backend);
typedef void (*destroy_backend_fn)(audio_backend *backend);
typedef bool (*setup_backend_context_fn)(void *ctx, unsigned sample_rate, bool mono, int resampler, float music_volume,
                                         float sound_volume);
typedef void (*close_backend_context_fn)(void *ctx);

// Master volume setters (0.0 ... 1.0).
typedef void (*set_backend_sound_volume_fn)(void *ctx, float volume);
typedef void (*set_backend_music_volume_fn)(void *ctx, float volume);

// Primitive per-channel ops. audio.c picks the channel and bounds-checks the values;
typedef bool (*play_pcm_sound_fn)(void *ctx, int channel, const sound_source *src, int volume, int panning,
                                  int fade_in_ms);
typedef bool (*is_channel_playing_fn)(void *ctx, int channel);
typedef void (*stop_channel_fn)(void *ctx, int channel);
typedef void (*fade_out_channel_fn)(void *ctx, int channel, int ms);
typedef void (*set_channel_panning_fn)(void *ctx, int channel, int panning);

// Music. Backend takes ownership of `src`.
typedef void (*play_music_fn)(void *ctx, const music_source *src);
typedef void (*stop_music_fn)(void *ctx);

struct audio_backend {
    is_backend_available_fn is_available;
    get_backend_description_fn get_description;
    get_backend_name_fn get_name;

    get_backend_sample_rates_fn get_sample_rates;
    get_backend_info_fn get_info;

    set_backend_sound_volume_fn set_sound_volume;
    set_backend_music_volume_fn set_music_volume;

    create_backend_fn create;
    destroy_backend_fn destroy;

    setup_backend_context_fn setup_context;
    close_backend_context_fn close_context;

    play_pcm_sound_fn play_pcm_sound;
    is_channel_playing_fn is_channel_playing;
    stop_channel_fn stop_channel;
    fade_out_channel_fn fade_out_channel;
    set_channel_panning_fn set_channel_panning;

    play_music_fn play_music;
    stop_music_fn stop_music;

    void *ctx;
};

#endif // AUDIO_BACKEND_H
