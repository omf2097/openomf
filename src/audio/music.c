#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#include <strings.h> // strcasecmp
#endif // __linux__
#include "resources/ids.h"
#include "audio/music.h"
#include "audio/audio.h"
#include "audio/sources/dumb_source.h"
#include "utils/log.h"
#ifdef USE_OGGVORBIS
#include "audio/sources/vorbis_source.h"
#endif // USE_OGGVORBIS

#ifdef STANDALONE_SERVER
int music_play(const char *filename) { return 0; }
void music_set_volume(float volume) {}
void music_stop() {}
int music_playing() { return 1; }
#else // STANDALONE_SERVER

static unsigned int _music_stream_id = 0;
static unsigned int _music_resource_id = 0;
static float _music_volume = VOLUME_DEFAULT;

int music_play(unsigned int id) {
    // Check if the wanted music is already playing
    if(id == _music_resource_id && _music_stream_id != 0) {
        return 0;
    }

    // ... Okay, it's not. Create a new resource and start loading.
    audio_source *music_src = malloc(sizeof(audio_source));
    source_init(music_src);

    // Find path & ext
    char* filename = get_path_by_id(id);
    char* ext = strrchr(filename, '.') + 1;
    if(ext == NULL || ext == filename) {
        PERROR("Couldn't find extension for music file!");
        goto error_0;
    }

    // Open correct source
    if(strcasecmp(ext, "psm") == 0) {
        dumb_source_init(music_src, filename);
    } else if(strcasecmp(ext, "s3m") == 0) {
        dumb_source_init(music_src, filename);
    } else if(strcasecmp(ext, "mod") == 0) {
        dumb_source_init(music_src, filename);
    } else if(strcasecmp(ext, "it") == 0) {
        dumb_source_init(music_src, filename);
    } else if(strcasecmp(ext, "xm") == 0) {
        dumb_source_init(music_src, filename);
    }
#ifdef USE_OGGVORBIS
    else if(strcasecmp(ext, "ogg") == 0) {
        vorbis_source_init(music_src, filename);
    }
#endif // USE_OGGVORBIS
    else {
        PERROR("No suitable music streamer found for format '%s'.", ext);
        goto error_0;
    }

    // Source settings
    source_set_loop(music_src, 1);

    // Stop previous
    music_stop();

    // Start playback
    _music_resource_id = id;
    _music_stream_id = sink_play(audio_get_sink(), music_src);
    sink_set_stream_volume(audio_get_sink(), _music_stream_id, _music_volume);

    // All done
    free(filename);
    return 0;

error_0:
    free(filename);
    free(music_src);
    return 1;
}

void music_set_volume(float volume) {
    _music_volume = volume;
    if(_music_stream_id != 0) {
        sink_set_stream_volume(audio_get_sink(), _music_stream_id, _music_volume);
    }
}

void music_stop() {
    if(_music_stream_id == 0) {
        return;
    }
    sink_stop(audio_get_sink(), _music_stream_id);
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
