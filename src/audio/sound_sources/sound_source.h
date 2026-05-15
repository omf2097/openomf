/**
 * @file sound_source.h
 * @brief Pre-decoded PCM sample descriptor (sound counterpart to music_source)
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef SOUND_SOURCE_H
#define SOUND_SOURCE_H

#include <stddef.h>

typedef struct sound_source sound_source;

struct sound_source {
    const char *buf;
    size_t len;
    int freq;

    void *context; ///< per-loader state or NULL
    void (*close)(sound_source *self);
};

static inline void sound_source_close(sound_source *src) {
    if(src != NULL && src->close != NULL) {
        src->close(src);
        src->close = NULL;
    }
}

#endif // SOUND_SOURCE_H
