#include <stdlib.h>
#include "audio/audio.h"
#include "audio/source.h"
#include "audio/sources/raw_source.h"
#include "audio/sink.h"
#include "audio/sound.h"
#include "resources/sounds_loader.h"

static float _sound_volume = VOLUME_DEFAULT;

#ifdef STANDALONE_SERVER
unsigned int sound_play(int id, float volume, float panning, float pitch) {}
#else
unsigned int sound_play(int id, float volume, float panning, float pitch) {
    audio_sink *sink = audio_get_sink();

    // If there is no sink, do nothing
    if(sink == NULL) {
        return -1;
    }

    // Get sample data
    char *buf;
    int len;
    if(sounds_loader_get(id, &buf, &len) != 0) {
        return -1;
    }

    // Play
    audio_source *src = malloc(sizeof(audio_source));
    source_init(src);
    raw_source_init(src, buf, len);
    unsigned int sound_id = sink_play_set(sink, src, volume, panning, pitch);
    sink_set_stream_volume(sink, sound_id, _sound_volume);
    return sound_id;
}
#endif

int sound_playing(unsigned int sound_id) {
    audio_sink *sink = audio_get_sink();
    if(sink == NULL) {
        return 0;
    }
    return sink_is_playing(sink, sound_id);
}

void sound_set_volume(float volume) {
    _sound_volume = volume;
}
