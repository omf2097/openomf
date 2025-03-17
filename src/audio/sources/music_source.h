#ifndef MUSIC_SOURCE_H
#define MUSIC_SOURCE_H

#include "SDL_mixer.h"

typedef void (*music_render)(void *ctx, char *data, int len);
typedef void (*music_close)(void *ctx);

typedef struct music_source {
    void *context;
    music_render render;
    music_close close;
} music_source;

static inline void music_source_render(music_source *src, char *data, int len) {
    if(src->context != NULL) {
        src->render(src->context, data, len);
    }
}

static inline void music_source_close(music_source *src) {
    if (src->context != NULL) {
        src->close(src->context);
        src->context = NULL;
    }
}

#endif // MUSIC_SOURCE_H
