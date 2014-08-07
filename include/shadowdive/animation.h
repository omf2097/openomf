#ifndef _SD_ANIMATION_H
#define _SD_ANIMATION_H

#include <stdint.h>
#include "shadowdive/sprite.h"
#include "shadowdive/colcoord.h"
#ifdef SD_USE_INTERNAL
    #include "shadowdive/internal/reader.h"
    #include "shadowdive/internal/writer.h"
#endif

#ifdef __cplusplus 
extern "C" {
#endif

#define SD_ANIMATION_STRING_MAX 1024
#define SD_EXTRA_STRING_MAX 512

#define SD_SPRITE_COUNT_MAX 255
#define SD_COLCOORD_COUNT_MAX 128
#define SD_EXTRASTR_COUNT_MAX 10

typedef struct {
    // Header
    int16_t start_x;
    int16_t start_y;
    int32_t null;
    uint16_t coord_count;
    uint8_t sprite_count;
    uint8_t extra_string_count;

    // Sprites and their collision coordinates
    sd_coord coord_table[SD_COLCOORD_COUNT_MAX];
    sd_sprite *sprites[SD_SPRITE_COUNT_MAX];

    // String header & Extra strings
    char anim_string[SD_ANIMATION_STRING_MAX];
    char extra_strings[SD_EXTRASTR_COUNT_MAX][SD_EXTRA_STRING_MAX];
} sd_animation;

int sd_animation_create(sd_animation* animation);
int sd_animation_copy(sd_animation *dst, const sd_animation *src);
void sd_animation_free(sd_animation *animation);

int sd_animation_set_anim_string(sd_animation *animation, const char *str);

int sd_animation_get_extra_string_count(sd_animation *animation);
int sd_animation_set_extra_string(sd_animation *animation, int num, const char *str);
int sd_animation_push_extra_string(sd_animation *anim, const char *str);
int sd_animation_pop_extra_string(sd_animation *anim);
char* sd_animation_get_extra_string(sd_animation *animation, int num);

int sd_animation_get_sprite_count(sd_animation *animation);
int sd_animation_set_sprite(sd_animation *animation, int num, const sd_sprite *sprite);
int sd_animation_push_sprite(sd_animation *animation, const sd_sprite *sprite);
int sd_animation_pop_sprite(sd_animation *animation);
sd_sprite* sd_animation_get_sprite(sd_animation *animation, int num);

#ifdef SD_USE_INTERNAL
int sd_animation_load(sd_reader *reader, sd_animation *animation);
int sd_animation_save(sd_writer *writer, const sd_animation *animation);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_ANIMATION_H
