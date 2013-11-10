#include <stdlib.h>
#include <string.h>
#include "audio/music.h"
#include "audio/audio.h"
#include "audio/sources/dumb_source.h"
#include "utils/log.h"
#ifdef USE_OGGVORBIS
#include "audio/sources/vorbis_source.h"
#endif // USE_OGGVORBIS

unsigned int music_id = 0;

int music_play(const char *filename) {
    audio_source *music_src = malloc(sizeof(audio_source));
    source_init(music_src);

    // Find ext
    char *ext = strrchr(filename, '.') + 1;
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
        PERROR("No suitable music streamer found.");
        goto error_0;
    }

    // Source settings
    source_set_loop(music_src, 1);

    // Start playback
    music_id = sink_play(audio_get_sink(), music_src);

    // All done
    return 0;
error_0:
    free(music_src);
    return 1;
}

void music_stop() {
    if(music_id == 0) return;
    sink_stop(audio_get_sink(), music_id);
    music_id = 0;
}

int music_playing() {
    if(music_id == 0) {
        return 0;
    }

    return 1;
}