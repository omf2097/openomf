#include "audio/backends/null/null_backend.h"
#include "audio/backends/audio_backend.h"
#include "utils/c_array_util.h"
#include "utils/log.h"

#include <assert.h>

#define CHANNEL_MAX 8

static const audio_sample_rate supported_sample_rates[] = {
    {44100, 1, "44100Hz"},
};
static const int supported_sample_rate_count = N_ELEMENTS(supported_sample_rates);

static const audio_resampler supported_resamplers[] = {
    {0, 1, "Linear"},
};
static const int supported_resamplers_count = N_ELEMENTS(supported_resamplers);

static bool is_available(void) {
    return true; // This is always available if compiled in.
}

static const char *get_description(void) {
    return "NULL audio output";
}

static const char *get_name(void) {
    return "NULL";
}

static unsigned int get_sample_rates(const audio_sample_rate **sample_rates) {
    *sample_rates = supported_sample_rates;
    return supported_sample_rate_count;
}

static unsigned int get_resamplers(const audio_resampler **resamplers) {
    *resamplers = supported_resamplers;
    return supported_resamplers_count;
}

static void create_backend(audio_backend *player) {
}

static void destroy_backend(audio_backend *player) {
}

static void set_backend_sound_volume(void *userdata, float volume) {
}

static void set_backend_music_volume(void *userdata, float volume) {
}

static void play_sound(void *userdata, const char *src_buf, size_t src_len, float volume, float panning, float pitch) {
}

static void stop_music(void *ctx) {
}

static void play_music(void *userdata, const char *file_name) {
}

static bool setup_backend_context(void *userdata, unsigned sample_rate, bool mono, unsigned resampler,
                                  float music_volume, float sound_volume) {
    INFO("NULL Player initialized!");
    return true;
}

static void close_backend_context(void *userdata) {
    INFO("NULL Renderer closed!");
}

void null_audio_backend_set_callbacks(audio_backend *sdl_backend) {
    sdl_backend->is_available = is_available;
    sdl_backend->get_description = get_description;
    sdl_backend->get_name = get_name;
    sdl_backend->get_sample_rates = get_sample_rates;
    sdl_backend->get_resamplers = get_resamplers;
    sdl_backend->create = create_backend;
    sdl_backend->destroy = destroy_backend;
    sdl_backend->set_music_volume = set_backend_music_volume;
    sdl_backend->set_sound_volume = set_backend_sound_volume;
    sdl_backend->setup_context = setup_backend_context;
    sdl_backend->close_context = close_backend_context;
    sdl_backend->play_music = play_music;
    sdl_backend->play_sound = play_sound;
    sdl_backend->stop_music = stop_music;
}
