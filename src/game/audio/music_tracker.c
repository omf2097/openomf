#include "game/audio/music_tracker.h"
#include "audio/audio.h"
#include "game/audio/audio_sources.h"
#include "resources/ids.h"

static resource_id current_music = NUMBER_OF_RESOURCES;

void music_tracker_play(const resource_id id) {
    if(current_music == id) {
        return;
    }
    music_source music;
    unsigned channels, sample_rate, resampler;
    audio_get_music_info(&sample_rate, &channels, &resampler);
    if(music_source_pick(&music, id, channels, sample_rate, resampler)) {
        audio_play_music(&music);
    }
    current_music = id;
}

void music_tracker_stop(void) {
    if(current_music == NUMBER_OF_RESOURCES) {
        return;
    }
    audio_stop_music();
    current_music = NUMBER_OF_RESOURCES;
}
