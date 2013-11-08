#include <stdlib.h>
#include "audio/audio.h"
#include "audio/source.h"
#include "audio/sources/raw_source.h"
#include "audio/sink.h"
#include "audio/sound.h"
#include "resources/sounds_loader.h"

void sound_play(int id) {
    // Get sample data
    char *buf;
    int len;
    if(sounds_loader_get(id, &buf, &len) != 0) {
        return;
    }

    // Play
    audio_source *src = malloc(sizeof(audio_source));
    source_init(src);
    raw_source_init(src, buf, len);
    sink_play(audio_get_sink(), src);
}

