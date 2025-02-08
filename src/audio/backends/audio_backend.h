#ifndef AUDIO_BACKEND_H
#define AUDIO_BACKEND_H

#include <stdbool.h>
#include <stddef.h>

#define VOLUME_DEFAULT 1.0f
#define PANNING_DEFAULT 0.0f
#define PITCH_DEFAULT 1.0f

#define VOLUME_MAX 1.0f
#define PANNING_MAX 1.0f
#define PITCH_MAX 2.0f

#define VOLUME_MIN 0.0f
#define PANNING_MIN -1.0f
#define PITCH_MIN 0.5f

typedef struct audio_resampler {
    int internal_id;
    bool is_default;
    const char *name;
} audio_resampler;

typedef struct audio_sample_rate {
    unsigned int sample_rate;
    bool is_default;
    const char *name;
} audio_sample_rate;

typedef struct audio_backend audio_backend;

// Metadata functions, all must be implemented. These must NOT require context or renderer state to be initialized!
typedef bool (*is_backend_available_fn)(void);
typedef const char *(*get_backend_description_fn)(void);
typedef const char *(*get_backend_name_fn)(void);

// Player available settings getters
typedef unsigned int (*get_backend_sample_rates_fn)(const audio_sample_rate **sample_rates);
typedef unsigned int (*get_backend_resamplers_fn)(const audio_resampler **resamplers);

// These initialize the player itself; they should be used to reserve and free internal context objects.
typedef void (*create_backend_fn)(audio_backend *renderer);
typedef void (*destroy_backend_fn)(audio_backend *renderer);

// Volume setters
typedef void (*set_backend_sound_volume_fn)(void *ctx, float volume);
typedef void (*set_backend_music_volume_fn)(void *ctx, float volume);

// Renderer initialization and de-initialization, these must be implemented.
typedef bool (*setup_backend_context_fn)(void *ctx, unsigned sample_rate, bool mono, unsigned resampler,
                                         float music_volume, float sound_volume);
typedef void (*close_backend_context_fn)(void *ctx);

// Playback handling.
typedef int (*play_sound_fn)(void *ctx, const char *buf, size_t len, float volume, float panning, float pitch,
                             int fade);
typedef void (*play_music_fn)(void *ctx, const char *file_name);
typedef void (*stop_music_fn)(void *ctx);

typedef void (*fade_out_fn)(int playback_id, int ms);

struct audio_backend {
    is_backend_available_fn is_available;
    get_backend_description_fn get_description;
    get_backend_name_fn get_name;

    get_backend_sample_rates_fn get_sample_rates;
    get_backend_resamplers_fn get_resamplers;

    set_backend_sound_volume_fn set_sound_volume;
    set_backend_music_volume_fn set_music_volume;

    create_backend_fn create;
    destroy_backend_fn destroy;

    setup_backend_context_fn setup_context;
    close_backend_context_fn close_context;

    play_sound_fn play_sound;
    play_music_fn play_music;
    stop_music_fn stop_music;

    fade_out_fn fade_out;

    void *ctx;
};

#endif // AUDIO_BACKEND_H
