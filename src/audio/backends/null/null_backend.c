#include "audio/backends/null/null_backend.h"
#include "audio/backends/audio_backend.h"
#include "utils/c_array_util.h"
#include "utils/log.h"

#include <assert.h>
#include <string.h>

static const audio_sample_rate supported_sample_rates[] = {
    {48000, 1, "48000Hz"},
};
static const int supported_sample_rate_count = N_ELEMENTS(supported_sample_rates);

static bool is_available(void) {
    return true;
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

static void create_backend(audio_backend *player) {
}

static void destroy_backend(audio_backend *player) {
}

static void set_backend_sound_volume(void *userdata, float volume) {
}

static void set_backend_music_volume(void *userdata, float volume) {
}

static bool play_pcm_sound(void *userdata, int channel, const sound_source *src, int volume, int panning,
                           int fade_in_ms) {
    return false;
}

static bool is_channel_playing(void *userdata, int channel) {
    return false;
}

static void stop_channel(void *userdata, int channel) {
}

static void fade_out_channel(void *userdata, int channel, int ms) {
}

static void stop_music(void *userdata) {
}

static void play_music(void *userdata, const music_source *src) {
    music_source dst;
    memcpy(&dst, src, sizeof(music_source));
    music_source_close(&dst);
}

static void get_info(void *ctx, unsigned *sample_rate, unsigned *channels, unsigned *resampler) {
    if(sample_rate != NULL) {
        *sample_rate = 48000;
    }
    if(channels != NULL) {
        *channels = 2;
    }
    if(resampler != NULL) {
        *resampler = 1;
    }
}

static bool setup_backend_context(void *userdata, unsigned sample_rate, bool mono, int resampler, float music_volume,
                                  float sound_volume) {
    log_info("NULL Player initialized!");
    return true;
}

static void close_backend_context(void *userdata) {
    log_info("NULL Player closed!");
}

void null_audio_backend_set_callbacks(audio_backend *null_backend) {
    null_backend->is_available = is_available;
    null_backend->get_description = get_description;
    null_backend->get_name = get_name;
    null_backend->get_sample_rates = get_sample_rates;
    null_backend->get_info = get_info;
    null_backend->create = create_backend;
    null_backend->destroy = destroy_backend;
    null_backend->set_music_volume = set_backend_music_volume;
    null_backend->set_sound_volume = set_backend_sound_volume;
    null_backend->setup_context = setup_backend_context;
    null_backend->close_context = close_backend_context;
    null_backend->play_pcm_sound = play_pcm_sound;
    null_backend->is_channel_playing = is_channel_playing;
    null_backend->stop_channel = stop_channel;
    null_backend->fade_out_channel = fade_out_channel;
    null_backend->play_music = play_music;
    null_backend->stop_music = stop_music;
}
