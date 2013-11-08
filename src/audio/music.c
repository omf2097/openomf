#include <stdlib.h>
#include "audio/music.h"
#include "audio/audio.h"
#include "audio/sources/dumb_source.h"
#include "utils/log.h"

unsigned int music_id = 0;

int music_play(const char *filename) {
    audio_source *music_src = malloc(sizeof(audio_source));
    source_init(music_src);
    dumb_source_init(music_src, filename);
    music_id = sink_play(audio_get_sink(), music_src);

    // All done
    return 0;
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