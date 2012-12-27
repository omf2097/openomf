#ifndef ANIMATION_H
#define ANIMATION_H

#include "sprite.h"

typedef struct sd_animation_t {
    // Header
    char unknown_a[8];
    uint16_t overlay_count;
    uint8_t frame_count;
    uint32_t *overlay_table;

    // String header
    uint16_t anim_string_len;
    char *anim_string;
    uint8_t unknown_b;
    uint8_t extra_string_count;

    // Extra strings
    char **extra_strings;

    // Sprites
    sd_sprite **sprites;
} sd_animation;

sd_animation* sd_animation_create();
void sd_animation_delete(sd_animation *animation);

#endif // _ANIMATION_H
