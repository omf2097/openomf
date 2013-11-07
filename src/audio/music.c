#include <stdlib.h>
#include "audio/music.h"
#include "audio/audio.h"
#include "audio/sources/dumb_source.h"
#include "utils/log.h"

audio_source *music_src = NULL;

int music_play(const char *filename) {
    music_src = malloc(sizeof(audio_source));
    source_init(music_src);
    dumb_source_init(music_src, filename);
    sink_play(audio_get_sink(), music_src);

    // All done
    return 0;
}

void music_stop() {
    if(music_src == NULL) return;
    sink_stop(audio_get_sink(), music_src);
    source_free(music_src);
    free(music_src);
    music_src = NULL;
}

int music_playing() {
    if(music_src == NULL) {
        return 0;
    }

    return 1;
}