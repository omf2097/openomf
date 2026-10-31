#include "audio/audio.h"
#include "audio/backends/audio_backend.h"
#include "audio/sound_sources/dat_source.h"
#include "utils/c_array_util.h"
#include "utils/log.h"

#include <assert.h>
#include <string.h>

#ifdef ENABLE_SDL_AUDIO_BACKEND
#include "audio/backends/sdl/sdl_backend.h"
#endif
#ifdef ENABLE_NULL_AUDIO_BACKEND
#include "audio/backends/null/null_backend.h"
#endif

#define MAX_AVAILABLE_BACKENDS 8

// Playback handles are opaque: the low bits hold the channel, the rest a guid that
// is unique per play. This is used to validate the handle.
typedef union {
    struct {
        uint32_t channel : 2;
        uint32_t guid : 30;
    } fields;
    uint32_t bits;
} sound_handle;

static_assert(sizeof(sound_handle) == 4, "sound_handle must be 4 bytes");
static_assert(SOUND_CHANNEL_COUNT <= 4, "channel index must fit in sound_handle.channel (2 bits)");

typedef void (*audio_backend_init)(audio_backend *backend);

// All built-in backends, most preferred first.
static audio_backend_init all_backends[] = {
#ifdef ENABLE_SDL_AUDIO_BACKEND
    sdl_audio_backend_set_callbacks,
#endif
#ifdef ENABLE_NULL_AUDIO_BACKEND
    null_audio_backend_set_callbacks,
#endif
};
static int all_backends_count = N_ELEMENTS(all_backends);

// Backends present on this platform -- filled by audio_scan_backends.
static struct available_backend {
    audio_backend_init set_callbacks;
    const char *name;
    const char *description;
} available_backends[MAX_AVAILABLE_BACKENDS];
static int audio_backend_count = 0;

static audio_backend current_backend;

static struct channel_state {
    int priority;
    int sound_id;
    uint32_t guid;
} channel_state[SOUND_CHANNEL_COUNT];

static uint32_t next_guid = 1;

static void reset_channel_state(void) {
    memset(channel_state, 0, sizeof(channel_state));
}

static int resolve_handle(const uint32_t handle) {
    const sound_handle decoded = {.bits = handle};
    const int ch = decoded.fields.channel;
    const uint32_t guid = decoded.fields.guid;
    if(ch >= SOUND_CHANNEL_COUNT || channel_state[ch].guid != guid) {
        return -1;
    }
    return ch;
}

void audio_scan_backends(void) {
    audio_backend tmp;
    audio_backend_count = 0;
    for(int i = 0; i < all_backends_count; i++) {
        all_backends[i](&tmp);
        if(audio_backend_count >= MAX_AVAILABLE_BACKENDS) {
            break;
        }
        if(tmp.is_available()) {
            available_backends[audio_backend_count].set_callbacks = all_backends[i];
            available_backends[audio_backend_count].name = tmp.get_name();
            available_backends[audio_backend_count].description = tmp.get_description();
            log_debug("Audio backend '%s' is available", available_backends[audio_backend_count].name);
            audio_backend_count++;
        }
    }
}

bool audio_get_backend_info(const int index, const char **name, const char **description) {
    if(index < 0 || index >= audio_backend_count) {
        return false;
    }
    if(name != NULL) {
        *name = available_backends[index].name;
    }
    if(description != NULL) {
        *description = available_backends[index].description;
    }
    return true;
}

int audio_get_backend_count(void) {
    return audio_backend_count;
}

static bool hunt_backend_by_name(const char *try_name) {
    for(int i = 0; i < audio_backend_count; i++) {
        if(strcmp(available_backends[i].name, try_name) != 0) {
            continue;
        }
        available_backends[i].set_callbacks(&current_backend);
        return true;
    }
    return false;
}

static bool find_best_backend(void) {
    if(audio_backend_count > 0) {
        available_backends[0].set_callbacks(&current_backend);
        return true;
    }
    return false;
}

static bool audio_find_backend(const char *try_name) {
    if(try_name != NULL && strlen(try_name) > 0) {
        if(hunt_backend_by_name(try_name)) {
            log_info("Found configured audio backend '%s'!", current_backend.get_name());
            return true;
        }
        log_error("Unable to find specified audio backend '%s', trying other alternatives ...", try_name);
    }
    if(find_best_backend()) {
        log_info("Found available audio backend '%s'!", current_backend.get_name());
        return true;
    }
    log_error("Unable to find any available audio backend!");
    memset(&current_backend, 0, sizeof(audio_backend));
    return false;
}

bool audio_init(const char *try_name, const int sample_rate, const bool mono, const int resampler,
                const float music_volume, const float sound_volume) {
    if(!audio_find_backend(try_name)) {
        goto exit_0;
    }
    current_backend.create(&current_backend);
    if(!current_backend.setup_context(current_backend.ctx, sample_rate, mono, resampler, music_volume, sound_volume)) {
        goto exit_1;
    }
    reset_channel_state();
    return true;

exit_1:
    current_backend.destroy(&current_backend);
exit_0:
    return false;
}

void audio_close(void) {
    current_backend.close_context(current_backend.ctx);
    current_backend.destroy(&current_backend);
    reset_channel_state();
}

uint32_t audio_play_sound_simple(const int sound_id, const int panning) {
    sound_source src;
    if(!dat_source_load(&src, sound_id)) {
        log_error("Requested sound sample %d not found or empty", sound_id);
        return AUDIO_INVALID_HANDLE;
    }
    sound_opts opts;
    sound_opts_init(&opts);
    opts.volume = 64;
    opts.panning = panning;
    const uint32_t result = audio_play_source(&src, &opts);
    sound_source_close(&src);
    return result;
}

static int pick_channel(const sound_opts *opts, const int identity) {
    // If channel is forced, just handle that.
    if(opts->channel >= 0) {
        if(current_backend.is_channel_playing(current_backend.ctx, opts->channel)) {
            log_debug("ch=%d: forced replace (was id=%d prio=%d)", opts->channel, channel_state[opts->channel].sound_id,
                      channel_state[opts->channel].priority);
            current_backend.stop_channel(current_backend.ctx, opts->channel);
        }
        return opts->channel;
    }

    // Duplicate handling only applies on the auto path and only when the
    // source has an identity. We scan and act on the first matching channel.
    if(identity != 0 && (opts->skip_duplicate || opts->stop_duplicate)) {
        for(int ch = 0; ch < SOUND_CHANNEL_COUNT; ch++) {
            if(!current_backend.is_channel_playing(current_backend.ctx, ch)) {
                continue;
            }
            if(channel_state[ch].sound_id != identity) {
                continue;
            }
            if(opts->skip_duplicate) {
                log_debug("Dropping sound id=%d: already playing on ch=%d (skip_dup)", identity, ch);
                return -1;
            }
            log_debug("ch=%d: stop_dup replace (id=%d)", ch, identity);
            current_backend.stop_channel(current_backend.ctx, ch);
            return ch;
        }
    }

    // Prefer a free channel.
    for(int ch = 0; ch < SOUND_CHANNEL_COUNT; ch++) {
        if(!current_backend.is_channel_playing(current_backend.ctx, ch)) {
            return ch;
        }
    }

    for(int ch = 0; ch < SOUND_CHANNEL_COUNT; ch++) {
        if(channel_state[ch].priority <= opts->priority) {
            log_debug("ch=%d: evicting id=%d prio=%d for id=%d prio=%d", ch, channel_state[ch].sound_id,
                      channel_state[ch].priority, identity, opts->priority);
            current_backend.stop_channel(current_backend.ctx, ch);
            return ch;
        }
    }
    log_debug("Dropping sound id=%d prio=%d: no free channel and none evictable", identity, opts->priority);
    return -1;
}

uint32_t audio_play_source(const sound_source *src, const sound_opts *custom_opts) {
    if(src == NULL || src->buf == NULL || src->len == 0) {
        return AUDIO_INVALID_HANDLE;
    }
    sound_opts opts;
    if(custom_opts == NULL) {
        sound_opts_init(&opts);
    } else {
        opts = *custom_opts;
    }

    assert(opts.volume >= 0 && opts.volume <= 127);
    assert(opts.panning >= -100 && opts.panning <= 100);
    assert(opts.fade_in_ms >= 0);
    assert(opts.channel < SOUND_CHANNEL_COUNT);

    const int ch = pick_channel(&opts, src->sound_id);
    if(ch < 0) {
        return AUDIO_INVALID_HANDLE;
    }

    sound_source effective = *src;
    effective.freq = pitched_samplerate(src->freq, opts.pitch);
    log_debug("Playing sound id=%d on ch=%d: vol=%d pan=%d pitch=%d (freq %d->%d) prio=%d fade=%dms%s%s", src->sound_id,
              ch, opts.volume, opts.panning, opts.pitch, src->freq, effective.freq, opts.priority, opts.fade_in_ms,
              opts.skip_duplicate ? " skip_dup" : "", opts.stop_duplicate ? " stop_dup" : "");
    if(!current_backend.play_pcm_sound(current_backend.ctx, ch, &effective, opts.volume, opts.panning,
                                       opts.fade_in_ms)) {
        return AUDIO_INVALID_HANDLE;
    }
    const sound_handle handle = {
        {ch, next_guid++}
    };
    channel_state[ch].priority = opts.priority;
    channel_state[ch].sound_id = src->sound_id;
    channel_state[ch].guid = handle.fields.guid;
    return handle.bits;
}

void audio_fade_out(const uint32_t playback_id, const int ms) {
    const int ch = resolve_handle(playback_id);
    if(ch < 0) {
        return;
    }
    current_backend.fade_out_channel(current_backend.ctx, ch, ms);
}

void audio_set_pan(const uint32_t playback_id, const int panning) {
    int ch = resolve_handle(playback_id);
    if(ch < 0) {
        return;
    }
    current_backend.set_channel_panning(current_backend.ctx, ch, panning);
}

void audio_play_music(const music_source *src) {
    current_backend.play_music(current_backend.ctx, src);
}

void audio_stop_music(void) {
    current_backend.stop_music(current_backend.ctx);
}

void audio_get_music_info(unsigned *sample_rate, unsigned *channels, unsigned *resampler) {
    current_backend.get_info(current_backend.ctx, sample_rate, channels, resampler);
}

void audio_set_music_volume(const float volume) {
    current_backend.set_music_volume(current_backend.ctx, volume);
}

void audio_set_sound_volume(const float volume) {
    current_backend.set_sound_volume(current_backend.ctx, volume);
}

unsigned audio_get_sample_rates(const audio_sample_rate **sample_rates) {
    return current_backend.get_sample_rates(sample_rates);
}

int pitched_samplerate(const int src_freq, int pitch) {
    if(pitch < -20) {
        pitch = -20;
    }
    if(pitch > 0) {
        return src_freq + (src_freq * pitch * 3) / 100;
    }
    if(pitch < 0) {
        return src_freq + (src_freq * pitch * 2) / 100;
    }
    return src_freq;
}
