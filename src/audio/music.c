#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#include <strings.h> // strcasecmp
#endif // __linux__
#include "resources/pathmanager.h"
#include "audio/music.h"
#include "audio/audio.h"
#include "utils/log.h"
#include "game/utils/settings.h"

#include "audio/sources/dumb_source.h"
#include "audio/sources/modplug_source.h"
#include "audio/sources/xmp_source.h"
#include "audio/sources/vorbis_source.h"

#ifdef STANDALONE_SERVER
int music_play(const char *filename) { return 0; }
void music_set_volume(float volume) {}
void music_stop() {}
int music_playing() { return 1; }
#else // STANDALONE_SERVER

struct music_override_t {
    int id;
    const char *name;
};

static unsigned int _music_stream_id = 0;
static unsigned int _music_resource_id = 0;
static float _music_volume = VOLUME_DEFAULT;

const char* get_file_or_override(unsigned int id) {
    // Declare music overrides
    settings *s = settings_get();
    struct music_override_t overrides[] = {
        {PSM_ARENA0, s->sound.music_arena0},
        {PSM_ARENA1, s->sound.music_arena1},
        {PSM_ARENA2, s->sound.music_arena2},
        {PSM_ARENA3, s->sound.music_arena3},
        {PSM_ARENA4, s->sound.music_arena4},
        {PSM_MENU,   s->sound.music_menu},
        {PSM_END,    s->sound.music_end}
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

    int channels = settings_get()->sound.music_mono ? 1 : 2;
    (void)(channels);
    // Check if the wanted music is already playing
    if(id == _music_resource_id && _music_stream_id != 0) {
        return 0;
    }

    // ... Okay, it's not. Create a new resource and start loading.
    audio_source *music_src = malloc(sizeof(audio_source));
    source_init(music_src);

    // Find path & ext
    const char* filename = get_file_or_override(id);
    const char* ext = strrchr(filename, '.') + 1;
    if(ext == NULL || ext == filename) {
        PERROR("Couldn't find extension for music file!");
        goto error_0;
    }

    // Try to open as module file
    int failed = 1;
    if(strcasecmp(ext, "psm") == 0
       || strcasecmp(ext, "s3m") == 0
       || strcasecmp(ext, "mod") == 0
       || strcasecmp(ext, "it") == 0
       || strcasecmp(ext, "xm") == 0)
    {
#if USE_DUMB
        failed = dumb_source_init(music_src, filename, channels);
#elif USE_MODPLUG
        failed = modplug_source_init(music_src, filename, channels);
#elif USE_XMP
        failed = xmp_source_init(music_src, filename, channels);
#endif
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
    _music_stream_id = sink_play(sink, music_src);
    sink_set_stream_volume(sink, _music_stream_id, _music_volume);

    // All done
    return 0;

error_0:
    free(music_src);
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
    if(_music_stream_id != 0) {
        sink_set_stream_volume(sink, _music_stream_id, _music_volume);
    }
}

void music_stop() {
    audio_sink *sink = audio_get_sink();
    if(sink == NULL) {
        return;
    }
    if(_music_stream_id == 0) {
        return;
    }
    sink_stop(sink, _music_stream_id);
    _music_stream_id = 0;
}

int music_playing() {
    if(_music_stream_id == 0) {
        return 0;
    }
    return 1;
}

unsigned int music_get_resource() {
    return _music_resource_id;
}

#endif // STANDALONE_SERVER
