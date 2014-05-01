#include <stdlib.h>
#include "audio/audio.h"
#include "audio/source.h"
#include "audio/sources/raw_source.h"
#include "audio/sink.h"
#include "audio/sound.h"
#include "resources/sounds_loader.h"

static float _sound_volume = VOLUME_DEFAULT;

#ifdef STANDALONE_SERVER
void sound_play(int id, float volume, float panning, float pitch) {}
#else
void sound_play(int id, float volume, float panning, float pitch) {
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
    unsigned int sound_id = sink_play_set(audio_get_sink(), src, volume, panning, pitch);
    sink_set_stream_volume(audio_get_sink(), sound_id, _sound_volume);

}
#endif

void sound_set_volume(float volume) {
    _sound_volume = volume;
}
