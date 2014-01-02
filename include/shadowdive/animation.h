#ifndef _SD_ANIMATION_H
#define _SD_ANIMATION_H

#include "shadowdive/sprite.h"

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct col_coord_t {
    int16_t x;
    uint8_t x_ext; 
    uint8_t y_ext;
    int16_t y;
} col_coord;

typedef struct sd_animation_t {
    // Header
    int16_t start_x;
    int16_t start_y;
    char unknown_a[4];
    uint16_t col_coord_count;
    uint8_t frame_count;
    uint8_t extra_string_count;
    col_coord *col_coord_table;

    // String header
    char *anim_string;

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

#ifdef __cplusplus
}
#endif

#endif // _SD_ANIMATION_H
