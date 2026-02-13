#include "game/audio/audio_sources.h"
#include "game/utils/settings.h"
#include "audio/music_sources/opus_source.h"
#include "audio/music_sources/psm_source.h"
#include "audio/sound_sources/dat_source.h"
#include "resources/ids.h"
#include "resources/modmanager.h"
#include "resources/resource_files.h"
#include "utils/log.h"
#include "utils/path.h"
#include "utils/random.h"

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


    str fn;
    unsigned char *buf;
    size_t len;
    path music = get_resource_filename(get_resource_file(id));
    // check the modmanager here for a music mod
    path_stem(&music, &fn);
    int music_count = modmanager_count_music(&fn);
    int rand = rand_int(music_count + 1);
    int music_type = settings_get()->sound.music_type;

    if(music_count > 0 && music_type == 1 && rand == 0) {
        // remixes only, never select an original track (0)
        rand = rand_int(music_count) + 1;
    }
    if(music_type != 0 && rand && modmanager_get_music(&fn, rand - 1, &buf, &len)) {
        log_debug("found replacement music file for %s.PSM", str_c(&fn));
        str_free(&fn);
        return opus_load_memory(src, (int)channels, (int)sample_rate, buf, len);
    }
    str_free(&fn);
    log_debug("Found original music file %s", path_c(&music));
    return psm_load(src, channels, sample_rate, resampler, path_c(&music));
}
