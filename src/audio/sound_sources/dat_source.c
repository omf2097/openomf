#include "audio/sound_sources/dat_source.h"
#include "resources/sounds_loader.h"

bool dat_source_load(sound_source *src, const int sound_id) {
    char *buf;
    int len;
    int freq;
    if(!sounds_loader_get(sound_id, &buf, &len, &freq) || len <= 0) {
        return false;
    }
    src->buf = buf;
    src->len = (size_t)len;
    src->freq = freq;
    src->sound_id = sound_id;
    src->context = NULL;
    src->close = NULL; // borrowed view; nothing to release.
    return true;
}
