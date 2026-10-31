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

// Used by tests
static struct {
    bool playing;
    int last_sound_id;
    int last_pan;
    int play_count;
    int stop_count;
    int fade_count;
    int pan_update_count;
} channel_spy[SOUND_CHANNEL_COUNT];

void null_audio_backend_reset_state(void) {
    memset(channel_spy, 0, sizeof(channel_spy));
    for(int i = 0; i < SOUND_CHANNEL_COUNT; i++) {
        channel_spy[i].last_sound_id = -1;
    }
}

int null_audio_backend_get_play_count(const int channel) {
    assert(channel >= 0 && channel < SOUND_CHANNEL_COUNT);
    return channel_spy[channel].play_count;
}

int null_audio_backend_get_fade_count(const int channel) {
    assert(channel >= 0 && channel < SOUND_CHANNEL_COUNT);
    return channel_spy[channel].fade_count;
}

int null_audio_backend_get_stop_count(const int channel) {
    assert(channel >= 0 && channel < SOUND_CHANNEL_COUNT);
    return channel_spy[channel].stop_count;
}

int null_audio_backend_get_last_sound_id(const int channel) {
    assert(channel >= 0 && channel < SOUND_CHANNEL_COUNT);
    return channel_spy[channel].last_sound_id;
}

int null_audio_backend_get_last_pan(const int channel) {
    assert(channel >= 0 && channel < SOUND_CHANNEL_COUNT);
    return channel_spy[channel].last_pan;
}

int null_audio_backend_get_pan_update_count(const int channel) {
    assert(channel >= 0 && channel < SOUND_CHANNEL_COUNT);
    return channel_spy[channel].pan_update_count;
}

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

static void set_backend_sound_volume(void *userdata, const float volume) {
}

static void set_backend_music_volume(void *userdata, const float volume) {
}

static bool play_pcm_sound(void *userdata, const int channel, const sound_source *src, const int volume,
                           const int panning, const int fade_in_ms) {
    assert(channel >= 0 && channel < SOUND_CHANNEL_COUNT);
    channel_spy[channel].playing = true;
    channel_spy[channel].last_sound_id = src->sound_id;
    channel_spy[channel].last_pan = panning;
    channel_spy[channel].play_count++;
    return true;
}

static void set_channel_panning(void *userdata, const int channel, const int panning) {
    assert(channel >= 0 && channel < SOUND_CHANNEL_COUNT);
    channel_spy[channel].last_pan = panning;
    channel_spy[channel].pan_update_count++;
}

static bool is_channel_playing(void *userdata, const int channel) {
    assert(channel >= 0 && channel < SOUND_CHANNEL_COUNT);
    return channel_spy[channel].playing;
}

static void stop_channel(void *userdata, const int channel) {
    assert(channel >= 0 && channel < SOUND_CHANNEL_COUNT);
    if(channel_spy[channel].playing) {
        channel_spy[channel].stop_count++;
    }
    channel_spy[channel].playing = false;
}

static void fade_out_channel(void *userdata, const int channel, const int ms) {
    assert(channel >= 0 && channel < SOUND_CHANNEL_COUNT);
    channel_spy[channel].fade_count++;
    channel_spy[channel].playing = false;
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

static bool setup_backend_context(void *userdata, const unsigned sample_rate, const bool mono, const int resampler,
                                  const float music_volume, const float sound_volume) {
    null_audio_backend_reset_state();
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
    null_backend->set_channel_panning = set_channel_panning;
    null_backend->play_music = play_music;
    null_backend->stop_music = stop_music;
}
