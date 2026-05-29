#include "audio/music_sources/null_music_source.h"

#include <string.h>

static char null_music_sentinel;

static void null_render(void *ctx, char *data, const int len) {
    memset(data, 0, (size_t)len);
}

static void null_set_volume(void *ctx, const float volume) {
}

static void null_close(void *ctx) {
}

bool null_music_source_load(music_source *src) {
    src->context = &null_music_sentinel;
    src->render = null_render;
    src->set_volume = null_set_volume;
    src->close = null_close;
    return true;
}
