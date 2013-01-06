#ifndef _SD_ANIMATION_H
#define _SD_ANIMATION_H

#include "shadowdive/sprite.h"

typedef struct sd_reader_t sd_reader;
typedef struct sd_writer_t sd_writer;

typedef struct sd_animation_t {
    // Header
    int16_t start_x;
    int16_t start_y;
    char unknown_a[4];
    uint16_t overlay_count;
    uint8_t frame_count;
    uint32_t *overlay_table;

    // String header
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
int sd_animation_load(sd_reader *reader, sd_animation *anim);
void sd_animation_save(sd_writer *writer, sd_animation *anim);

void sd_animation_set_anim_string(sd_animation *animation, const char *str);
void sd_animation_set_extra_string(sd_animation *animation, int num, const char *str);

#endif // _SD_ANIMATION_H
