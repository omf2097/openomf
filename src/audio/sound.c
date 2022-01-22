#include "audio/sound.h"
#include "audio/audio.h"
#include "audio/sink.h"
#include "audio/source.h"
#include "audio/sources/raw_source.h"
#include "resources/sounds_loader.h"
#include "utils/allocator.h"

static float _sound_volume = VOLUME_DEFAULT;

void sound_play(int id, float volume, float panning, float pitch) {
    audio_sink *sink = audio_get_sink();

    // If there is no sink, do nothing
    if(sink == NULL) {
        return;
    }

    // If the sound is already playing, stop it.
    if(sink_is_playing(sink, id)) {
        sink_stop(sink, id);
    }

    // Get sample data
    char *buf;
    int len;
    if(sounds_loader_get(id, &buf, &len) != 0) {
        return;
    }

    // Play
    audio_source *src = omf_calloc(1, sizeof(audio_source));
    source_init(src);
    raw_source_init(src, buf, len);
    sink_play(sink, src, id, volume * _sound_volume, panning, pitch);
}

int sound_playing(unsigned int id) {
    audio_sink *sink = audio_get_sink();
    if(sink == NULL) {
        return 0;
    }
    return sink_is_playing(sink, id);
}

void sound_set_volume(float volume) {
    _sound_volume = volume;
}
