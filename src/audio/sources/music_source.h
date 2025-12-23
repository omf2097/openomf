#ifndef MUSIC_SOURCE_H
#define MUSIC_SOURCE_H

#include <stdbool.h>
#include <stddef.h>

typedef void (*music_render)(void *ctx, char *data, int len);
typedef void (*music_set_volume)(void *ctx, float volume);
typedef void (*music_close)(void *ctx);

typedef struct music_resampler {
    int internal_id;
    bool is_default;
    const char *name;
} music_resampler;

typedef struct music_source {
    void *context;
    music_render render;
    music_set_volume set_volume;
    music_close close;
} music_source;

static inline void music_source_render(music_source *src, char *data, int len) {
    if(src->context != NULL) {
        src->render(src->context, data, len);
    }
}

static inline void music_source_set_volume(music_source *src, float volume) {
    if(src->context != NULL) {
        src->set_volume(src->context, volume);
    }
}

static inline void music_source_close(music_source *src) {
    if(src->context != NULL) {
        src->close(src->context);
        src->context = NULL;
    }
}

#endif // MUSIC_SOURCE_H
