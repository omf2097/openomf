#ifndef _SD_ANIMATION_H
#define _SD_ANIMATION_H

#ifdef USE_INTERNAL
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#endif

#include "shadowdive/sprite.h"

#ifdef __cplusplus 
extern "C" {
#endif

#define SD_ANIMATION_STRING_MAX 1024
#define SD_EXTRA_STRING_MAX 512

#define SD_SPRITE_COUNT_MAX 255
#define SD_SPRITE_COLCOORD_COUNT_MAX 128
#define SD_SPRITE_EXTRASTR_COUNT_MAX 10

typedef struct {
    // Header
    int16_t start_x;
    int16_t start_y;
    int32_t null;
    uint16_t col_coord_count;
    uint8_t sprite_count;
    uint8_t extra_string_count;

    // Sprites and their collision coordinates
    col_coord col_coord_table[SD_SPRITE_COLCOORD_COUNT_MAX];
    sd_sprite *sprites[SD_SPRITE_COUNT_MAX];

    // String header & Extra strings
    char anim_string[SD_ANIMATION_STRING_MAX];
    char extra_strings[SD_SPRITE_EXTRASTR_COUNT_MAX][SD_EXTRA_STRING_MAX];
} sd_animation;

int sd_animation_create(sd_animation* animation);
int sd_animation_copy(sd_animation *animation, const sd_animation *src);
void sd_animation_free(sd_animation *animation);

int sd_animation_set_anim_string(sd_animation *animation, const char *str);
int sd_animation_set_extra_string(sd_animation *animation, int num, const char *str);

int sd_animation_set_sprite(sd_animation *animation, int num, const sd_sprite *sprite);
int sd_animation_push_sprite(sd_animation *animation, const sd_sprite *sprite);
int sd_animation_pop_sprite(sd_animation *animation, sd_sprite *sprite);
sd_sprite* sd_animation_get_sprite(sd_animation *animation, int num);

#ifdef USE_INTERNAL
int sd_animation_load(sd_reader *reader, sd_animation *anim);
void sd_animation_save(sd_writer *writer, sd_animation *anim);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_ANIMATION_H
