#ifndef ANIMATION_H
#define ANIMATION_H

#include "resources/sprite.h"
#include "utils/array.h"
#include "utils/str.h"
#include "utils/vec.h"
#include "utils/vector.h"

typedef enum
{
    BK_ANIMATION,
    AF_ANIMATION
} animation_source;

// All HARs have these predefined animations
enum
{
    ANIM_JUMPING = 1,
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
    ANIM_VICTORY = 48,
    ANIM_DEFEAT,
    ANIM_BLAST1 = 55,
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
    uint8_t extra_string_count;
    vector extra_strings;
    vector sprites;
} animation;

void animation_create(animation_source type, int file_id, animation *ani, array *sprites, void *src, int id);
sprite *animation_get_sprite(animation *ani, int sprite_id);
void animation_free(animation *ani);

int animation_get_sprite_count(animation *ani);

animation *create_animation_from_single(sprite *sp, vec2i pos);
void animation_fixup_coordinates(animation *ani, int fix_x, int fix_y);

int animation_clone(animation *src, animation *dst);

#endif // ANIMATION_H
