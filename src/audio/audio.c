#include "audio/audio.h"
#include "audio/backends/audio_backend.h"
#include "audio/music_sources/opus_source.h"
#include "audio/music_sources/psm_source.h"
#include "audio/sound_sources/dat_source.h"
#include "resources/resource_files.h"
#include "utils/c_array_util.h"
#include "utils/log.h"
#include "utils/path.h"

#include <assert.h>

#ifdef ENABLE_SDL_AUDIO_BACKEND
#include "audio/backends/sdl/sdl_backend.h"
#endif
#ifdef ENABLE_NULL_AUDIO_BACKEND
#include "audio/backends/null/null_backend.h"
#endif

#define MAX_AVAILABLE_BACKENDS 8

typedef enum music_file_type
{
    MUSIC_FILE_TYPE_PSM,
    MUSIC_FILE_TYPE_OGG,
} music_file_type;

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

// Backends present on this platform — filled by audio_scan_backends.
static struct available_backend {
    audio_backend_init set_callbacks;
    const char *name;
    const char *description;
} available_backends[MAX_AVAILABLE_BACKENDS];
static int audio_backend_count = 0;

static audio_backend current_backend;
static resource_id current_music = NUMBER_OF_RESOURCES;

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

bool audio_get_backend_info(int index, const char **name, const char **description) {
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

bool audio_init(const char *try_name, int sample_rate, bool mono, int resampler, float music_volume,
                float sound_volume) {
    if(!audio_find_backend(try_name)) {
        goto exit_0;
    }
    current_backend.create(&current_backend);
    if(!current_backend.setup_context(current_backend.ctx, sample_rate, mono, resampler, music_volume, sound_volume)) {
        goto exit_1;
    }
    return true;

exit_1:
    current_backend.destroy(&current_backend);
exit_0:
    return false;
}

void audio_close(void) {
    current_backend.close_context(current_backend.ctx);
    current_backend.destroy(&current_backend);
    current_music = NUMBER_OF_RESOURCES;
}

int audio_play_sound(int sound_id, const sound_opts *opts) {
    sound_source src;
    if(!dat_source_load(&src, sound_id)) {
        log_error("Requested sound sample %d not found or empty", sound_id);
        return -1;
    }
    const int result = audio_play_source(&src, opts);
    sound_source_close(&src);
    return result;
}

int audio_play_source(const sound_source *src, const sound_opts *custom_opts) {
    if(src == NULL || src->buf == NULL || src->len == 0) {
        return -1;
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

    sound_source effective = *src;
    effective.freq = pitched_samplerate(src->freq, opts.pitch);
    for(int ch = 0; ch < SOUND_CHANNEL_COUNT; ch++) {
        if(current_backend.is_channel_playing(current_backend.ctx, ch)) {
            continue;
        }
        if(!current_backend.play_pcm_sound(current_backend.ctx, ch, &effective, opts.volume, opts.panning,
                                           opts.fade_in_ms)) {
            return -1;
        }
        return ch;
    }
    return -1;
}

void audio_fade_out(int playback_id, int ms) {
    if(playback_id < 0 || playback_id >= SOUND_CHANNEL_COUNT) {
        return;
    }
    current_backend.fade_out_channel(current_backend.ctx, playback_id, ms);
}

static void load_xmp_music(const char *src) {
    music_source music;
    unsigned channels;
    unsigned sample_rate;
    unsigned resampler;
    current_backend.get_info(current_backend.ctx, &sample_rate, &channels, &resampler);
    if(psm_load(&music, channels, sample_rate, resampler, src)) {
        current_backend.play_music(current_backend.ctx, &music);
    }
}

static void load_opus_music(const char *src) {
    music_source music;
    unsigned channels;
    unsigned sample_rate;
    current_backend.get_info(current_backend.ctx, &sample_rate, &channels, NULL);
    if(opus_load(&music, channels, sample_rate, src)) {
        current_backend.play_music(current_backend.ctx, &music);
    }
}

static path get_music_path(music_file_type *type, unsigned int resource_id) {
    assert(is_music(resource_id));
    path original_music, new_music;
    original_music = new_music = get_resource_filename(get_resource_file(resource_id));
    path_set_ext(&new_music, ".ogg");

    if(path_exists(&new_music)) {
        log_debug("Found alternate music file %s", path_c(&new_music));
        *type = MUSIC_FILE_TYPE_OGG;
        return new_music;
    } else {
        log_debug("Found original music file %s", path_c(&original_music));
        *type = MUSIC_FILE_TYPE_PSM;
        return original_music;
    }
}

void audio_play_music(resource_id id) {
    if(current_music != id) {
        music_file_type file_type;
        const path music = get_music_path(&file_type, id);

        switch(file_type) {
            case MUSIC_FILE_TYPE_PSM:
                load_xmp_music(path_c(&music));
                break;
            case MUSIC_FILE_TYPE_OGG:
                load_opus_music(path_c(&music));
                break;
            default:
                log_error("Unable to load music file %s due to unsupported audio format", path_c(&music));
                break;
        }

        current_music = id;
    }
}

void audio_stop_music(void) {
    if(current_music != NUMBER_OF_RESOURCES) {
        current_backend.stop_music(current_backend.ctx);
        current_music = NUMBER_OF_RESOURCES;
    }
}

void audio_set_music_volume(float volume) {
    current_backend.set_music_volume(current_backend.ctx, volume);
}

void audio_set_sound_volume(float volume) {
    current_backend.set_sound_volume(current_backend.ctx, volume);
}

unsigned audio_get_sample_rates(const audio_sample_rate **sample_rates) {
    return current_backend.get_sample_rates(sample_rates);
}

int pitched_samplerate(int src_freq, int pitch) {
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
