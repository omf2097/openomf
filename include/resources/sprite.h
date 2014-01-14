#ifndef _SPRITE_H
#define _SPRITE_H

#include "resources/palette.h"
#include "video/surface.h"
#include "utils/vec.h"

typedef struct sprite_t {
    int id;
    vec2i pos;
    surface *data;
    char *stencil;
} sprite;

void sprite_create(sprite *sp, void *src, int id);
void sprite_create_custom(sprite *sp, vec2i pos, surface *sur);
void sprite_free(sprite *sp);

vec2i sprite_get_size(sprite *s);
sprite* sprite_copy(sprite *src);

#endif // _SPRITE_H
