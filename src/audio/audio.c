#include "audio/audio.h"
#include "audio/backends/audio_backend.h"
#include "audio/sources/opus_source.h"
#include "audio/sources/psm_source.h"
#include "resources/pathmanager.h"
#include "resources/sounds_loader.h"
#include "utils/c_array_util.h"
#include "utils/log.h"
#include "utils/miscmath.h"

// If-def the includes here
#ifdef ENABLE_SDL_AUDIO_BACKEND
#include "audio/backends/sdl/sdl_backend.h"
#endif
#ifdef ENABLE_NULL_AUDIO_BACKEND
#include "audio/backends/null/null_backend.h"
#endif

#define MAX_AVAILABLE_BACKENDS 8

typedef void (*audio_backend_init)(audio_backend *backend);

// This is the list of all built-in backends.
// If-def the backends here. Most preferred backends at the top.
static audio_backend_init all_backends[] = {
#ifdef ENABLE_SDL_AUDIO_BACKEND
    sdl_audio_backend_set_callbacks,
#endif
#ifdef ENABLE_NULL_AUDIO_BACKEND
    null_audio_backend_set_callbacks,
#endif
};
static int all_backends_count = N_ELEMENTS(all_backends);

// All backends that are available. This is filled by audio_scan_backends().
static struct available_backend {
    audio_backend_init set_callbacks;
    const char *name;
    const char *description;
} available_backends[MAX_AVAILABLE_BACKENDS];
static int audio_backend_count = 0;

// Currently selected backend
static audio_backend current_backend;
static resource_id current_music = NUMBER_OF_RESOURCES;

/**
 * This is run at start to hunt the available audio backends.
 */
void audio_scan_backends(void) {
    audio_backend tmp;
    audio_backend_count = 0;
    for(int i = 0; i < all_backends_count; i++) {
        all_backends[i](&tmp);
        if(audio_backend_count >= MAX_AVAILABLE_BACKENDS)
            break;
        if(tmp.is_available()) {
            available_backends[audio_backend_count].set_callbacks = all_backends[i];
            available_backends[audio_backend_count].name = tmp.get_name();
            available_backends[audio_backend_count].description = tmp.get_description();
            log_debug("Audio backend '%s' is available", available_backends[audio_backend_count].name);
            audio_backend_count++;
        }
    }
}

/**
 * Get information about a audio backend by its index.
 * @param index Backend index
 * @param name Backend name
 * @param description Backend description
 * @return true if data was read, false if there was no backend at this index.
 */
bool audio_get_backend_info(int index, const char **name, const char **description) {
    if(index < 0 && index >= audio_backend_count)
        return false;
    if(name != NULL)
        *name = available_backends[index].name;
    if(description != NULL)
        *description = available_backends[index].description;
    return true;
}

/**
 * Get the number of currently available audio backends.
 * @return Number of available backends
 */
int audio_get_backend_count(void) {
    return audio_backend_count;
}

/**
 * Attempt to find the audio backend by name
 * @param try_name Backend name
 * @return true if backend was found, false if not.
 */
static bool hunt_backend_by_name(const char *try_name) {
    for(int i = 0; i < audio_backend_count; i++) {
        if(strcmp(available_backends[i].name, try_name) != 0)
            continue;
        available_backends[i].set_callbacks(&current_backend);
        return true;
    }
    return false;
}

/**
 * Attempt to find the best backend to use, if one is not selected.
 * We just assume the first available backend is the best one :)
 * @return true if backend was found, false if not.
 */
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
    if(!audio_find_backend(try_name))
        goto exit_0;
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

int audio_play_sound(int id, float volume, float panning, float pitch) {
    if(id < 0 || id > 299)
        return -1;

    // Load sample (8000Hz, mono, 8bit)
    char *src_buf;
    int src_len;
    if(!sounds_loader_get(id, &src_buf, &src_len)) {
        log_error("Requested sound sample %d not found", id);
        return -1;
    }
    if(src_len == 0) {
        log_debug("Requested sound sample %d has nothing to play", id);
        return -1;
    }

    // Tell the backend to play it.
    return current_backend.play_sound(current_backend.ctx, src_buf, src_len, volume, panning, pitch, 0);
}

int audio_play_sound_buf(char *src_buf, int src_len, float volume, float panning, float pitch, int fade) {
    // Tell the backend to play it.
    return current_backend.play_sound(current_backend.ctx, src_buf, src_len, volume, panning, pitch, fade);
}

void audio_fade_out(int playback_id, int ms) {
    current_backend.fade_out(playback_id, ms);
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

void audio_play_music(resource_id id) {
    if(current_music != id) {
        char path[1024];
        music_file_type file_type;
        pm_get_music_path(path, 1024, &file_type, id);

        switch(file_type) {
            case MUSIC_FILE_TYPE_PSM:
                load_xmp_music(path);
                break;
            case MUSIC_FILE_TYPE_OGG:
                load_opus_music(path);
                break;
            default:
                log_error("Unable to load music file %s due to unsupported audio format", path);
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

int pitched_samplerate(float pitch) {
    // all our audio is 8khz for now
    return (int)(SOURCE_FREQ * clampf(pitch, PITCH_MIN, PITCH_MAX));
}
