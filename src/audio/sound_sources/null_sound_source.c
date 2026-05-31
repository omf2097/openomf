#include "audio/sound_sources/null_sound_source.h"

#define NULL_SOUND_FREQ 8000
#define NULL_SOUND_MAX_ID 63
#define NULL_SOUND_BYTES_PER_ID 1000
#define NULL_SOUND_BUFFER_SIZE ((NULL_SOUND_MAX_ID + 1) * NULL_SOUND_BYTES_PER_ID)

static const char silence[NULL_SOUND_BUFFER_SIZE] = {0};

bool null_sound_source_load(sound_source *src, const int sound_id) {
    if(sound_id < 0 || sound_id > NULL_SOUND_MAX_ID) {
        return false;
    }
    src->buf = silence;
    src->len = (size_t)((sound_id + 1) * NULL_SOUND_BYTES_PER_ID);
    src->freq = NULL_SOUND_FREQ;
    src->sound_id = sound_id;
    src->context = NULL;
    src->close = NULL;
    return true;
}
