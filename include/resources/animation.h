#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "resources/sprite.h"
#include "utils/vec.h"
#include "utils/vector.h"
#include "utils/string.h"

// All HARs have these predefined animations
enum {
    ANIM_JUMPING=1,
    ANIM_STANDUP,
    ANIM_STUNNED,
    ANIM_CROUCHING,
    ANIM_STANDING_BLOCK,
    ANIM_CROUCHING_BLOCK,
    ANIM_BURNING_OIL,
    ANIM_BLOCKING_SCRAPE,
    ANIM_DAMAGE,
    ANIM_WALKING,
    ANIM_IDLE,
    ANIM_SCRAP_METAL,
    ANIM_BOLT,
    ANIM_SCREW,
    ANIM_VICTORY=48,
    ANIM_DEFEAT,
    ANIM_BLAST1=55,
    ANIM_BLAST2,
    ANIM_BLAST3
};

typedef struct collision_coord_t {
    vec2i pos;
    int frame_index;
} collision_coord;

typedef struct animation_t {
    int id;
    vec2i start_pos;
    vector collision_coords;
    str animation_string;
    vector extra_strings;
    vector sprites;
} animation;

void animation_create(animation *ani, void *src, int id);
sprite* animation_get_sprite(animation *ani, int sprite_id);
void animation_free(animation *ani);

animation* create_animation_from_single(sprite *sp, vec2i pos);

#endif // _ANIMATION_H