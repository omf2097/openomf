#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "resources/sprite.h"
#include "utils/vec.h"
#include "utils/vector.h"
#include "utils/string.h"

typedef struct collision_coord_t {
	vec2i pos;
	int frame_index;
} collision_coord;

typedef struct animation_t {
	vec2i start_pos;
	vector collision_coords;
	str animation_string;
	vector extra_strings;
	vector sprites;
} animation;

void animation_create(animation *ani, void *src);
sprite* animation_get_sprite(animation *ani, int sprite_id);
void animation_free(animation *ani);

animation* create_animation_from_single(sprite *sp, vec2i pos);

#endif // _ANIMATION_H