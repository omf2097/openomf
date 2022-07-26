#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#include <strings.h> // strcasecmp
#endif               // __linux__
#include "audio/audio.h"
#include "audio/music.h"
#include "game/utils/settings.h"
#include "resources/pathmanager.h"
#include "utils/allocator.h"
#include "utils/log.h"

#include "audio/sources/dumb_source.h"
#include "audio/sources/vorbis_source.h"
#include "audio/sources/xmp_source.h"

struct music_override_t {
    int id;
    const char *name;
};

#define MUSIC_STREAM_ID 1000

#define SOURCE_NONE 0
#define SOURCE_DUMB 1
#define SOURCE_XMP 2

static unsigned int _music_resource_id = 0;
static float _music_volume = VOLUME_DEFAULT;

static module_source module_sources[] = {
    {SOURCE_NONE, "none"},
#ifdef USE_DUMB
    {SOURCE_DUMB, "dumb"},
#endif
#ifdef USE_XMP
    {SOURCE_XMP,  "xmp" },
#endif
    {0,           0     }  // Guard
};

audio_source_freq default_freqs[] = {
    {0, 1, "none"},
    {0, 0 }
};

audio_source_resampler default_resamplers[] = {
    {0, 1, "default"},
    {0, 0 }
};

module_source *music_get_module_sources() {
    return module_sources;
}

const audio_source_freq *music_module_get_freqs(int id) {
    switch(id) {
#ifdef USE_DUMB
        case SOURCE_DUMB:
            return dumb_get_freqs();
#endif
#ifdef USE_XMP
        case SOURCE_XMP:
            return xmp_get_freqs();
#endif
    }
    return default_freqs;
}

const audio_source_resampler *music_module_get_resamplers(int id) {
    switch(id) {
#ifdef USE_DUMB
        case SOURCE_DUMB:
            return dumb_get_resamplers();
#endif
#ifdef USE_XMP
        case SOURCE_XMP:
            return xmp_get_resamplers();
#endif
    }
    return default_resamplers;
}

const char *get_file_or_override(unsigned int id) {
    // Declare music overrides
    settings *s = settings_get();
    struct music_override_t overrides[] = {
        {PSM_ARENA0, s->sound.music_arena0},
        {PSM_ARENA1, s->sound.music_arena1},
        {PSM_ARENA2, s->sound.music_arena2},
        {PSM_ARENA3, s->sound.music_arena3},
        {PSM_ARENA4, s->sound.music_arena4},
        {PSM_MENU,   s->sound.music_menu  },
        {PSM_END,    s->sound.music_end   }
    };

    for(int i = 0; i < 7; i++) {
        if(id == overrides[i].id && strlen(overrides[i].name) > 0) {
            DEBUG("Overriding %s to %s.", get_resource_name(id), overrides[i].name);
            return overrides[i].name;
        }
    }

    return pm_get_resource_path(id);
}

int music_play(unsigned int id) {
    audio_sink *sink = audio_get_sink();

    // If there is no sink, do nothing
    if(sink == NULL) {
        return 0;
    }

    // Get audio settings from config file
    int channels = settings_get()->sound.music_mono ? 1 : 2;
    int freq = settings_get()->sound.music_frequency;
    int modlib = settings_get()->sound.music_library;
    int resampler = settings_get()->sound.music_resampler;

    // Check if the wanted music is already playing
    if(id == _music_resource_id && sink_is_playing(sink, MUSIC_STREAM_ID)) {
        return 0;
    }

    // ... Okay, it's not. Create a new resource and start loading.
    audio_source *music_src = omf_calloc(1, sizeof(audio_source));
    source_init(music_src);

    // Find path & ext
    const char *filename = get_file_or_override(id);
    const char *ext = strrchr(filename, '.') + 1;
    if(ext == NULL || ext == filename) {
        PERROR("Couldn't find extension for music file!");
        goto error_0;
    }

    // Try to open as module file
    int failed = 1;
    if(strcasecmp(ext, "psm") == 0) {
        switch(modlib) {
#ifdef USE_DUMB
            case SOURCE_DUMB:
                failed = dumb_source_init(music_src, filename, channels, freq, resampler);
                break;
#endif
#ifdef USE_XMP
            case SOURCE_XMP:
                failed = xmp_source_init(music_src, filename, channels, freq, resampler);
                break;
#endif
        }
    }

    // Try to open as Ogg vorbis
#ifdef USE_OGGVORBIS
    if(strcasecmp(ext, "ogg") == 0) {
        failed = vorbis_source_init(music_src, filename);
    }
#endif // USE_OGGVORBIS

    // Handle opening failure
    if(failed) {
        PERROR("No suitable music streamer found for format '%s'.", ext);
        goto error_0;
    }

    // Source settings
    source_set_loop(music_src, 1);

    // Stop previous
    music_stop();

    // Start playback
    _music_resource_id = id;
    sink_play(sink, music_src, MUSIC_STREAM_ID, _music_volume, PANNING_DEFAULT, PITCH_DEFAULT);

    // All done
    return 0;

error_0:
    omf_free(music_src);
    return 1;
}

int music_reload() {
    unsigned int old_res_id = _music_resource_id;
    music_stop();
    return music_play(old_res_id);
}

void music_set_volume(float volume) {
    audio_sink *sink = audio_get_sink();
    if(sink == NULL) {
        return;
    }

    _music_volume = volume;
    if(sink_is_playing(sink, MUSIC_STREAM_ID)) {
        sink_set_stream_volume(sink, MUSIC_STREAM_ID, _music_volume);
    }
}

void music_stop() {
    audio_sink *sink = audio_get_sink();
    if(sink == NULL) {
        return;
    }
    if(!sink_is_playing(sink, MUSIC_STREAM_ID)) {
        return;
    }
    sink_stop(sink, MUSIC_STREAM_ID);
}

int music_playing() {
    audio_sink *sink = audio_get_sink();
    return sink_is_playing(sink, MUSIC_STREAM_ID);
}

unsigned int music_get_resource() {
    return _music_resource_id;
}
