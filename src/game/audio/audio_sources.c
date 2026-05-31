#include "game/audio/audio_sources.h"
#include "audio/music_sources/opus_source.h"
#include "audio/music_sources/psm_source.h"
#include "audio/sound_sources/dat_source.h"
#include "resources/ids.h"
#include "resources/resource_files.h"
#include "utils/log.h"
#include "utils/path.h"

#include <assert.h>

#ifdef ENABLE_NULL_AUDIO_SOURCES
#include "audio/music_sources/null_music_source.h"
#include "audio/sound_sources/null_sound_source.h"
#endif

static audio_sources_mode current_mode = AUDIO_SOURCES_REAL;

void audio_sources_set_mode(const audio_sources_mode mode) {
    current_mode = mode;
}

audio_sources_mode audio_sources_get_mode(void) {
    return current_mode;
}

bool sound_source_pick(sound_source *src, const int sound_id) {
#ifdef ENABLE_NULL_AUDIO_SOURCES
    if(current_mode == AUDIO_SOURCES_NULL) {
        return null_sound_source_load(src, sound_id);
    }
#endif
    return dat_source_load(src, sound_id);
}

bool music_source_pick(music_source *src, const resource_id id, const unsigned channels, const unsigned sample_rate,
                       const unsigned resampler) {
    assert(is_music(id));

#ifdef ENABLE_NULL_AUDIO_SOURCES
    if(current_mode == AUDIO_SOURCES_NULL) {
        return null_music_source_load(src);
    }
#endif

    path original_music, new_music;
    original_music = new_music = get_resource_filename(get_resource_file(id));
    path_set_ext(&new_music, ".ogg");

    if(path_exists(&new_music)) {
        log_debug("Found alternate music file %s", path_c(&new_music));
        return opus_load(src, (int)channels, (int)sample_rate, path_c(&new_music));
    }
    log_debug("Found original music file %s", path_c(&original_music));
    return psm_load(src, channels, sample_rate, resampler, path_c(&original_music));
}
