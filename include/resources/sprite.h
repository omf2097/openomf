#ifndef _SPRITE_H
#define _SPRITE_H

#include "resources/palette.h"
#include "video/texture.h"
#include "utils/vec.h"

typedef struct sprite_t {
	vec2i pos;
	void *raw_sprite;
} sprite;

void sprite_create(sprite *sp, void *src);
void sprite_create_custom(sprite *sp, vec2i pos, void *raw_sprite);
void sprite_free(sprite *sp);

sprite* sprite_copy(sprite *src);

#endif // _SPRITE_H
