#ifndef _SD_MOVE_H
#define _SD_MOVE_H

#include <stdint.h>
#include "shadowdive/animation.h"
#ifdef SD_USE_INTERNAL
    #include "shadowdive/internal/reader.h"
    #include "shadowdive/internal/writer.h"
#endif

#ifdef __cplusplus 
extern "C" {
#endif

#define SD_MOVE_STRING_MAX 21
#define SD_MOVE_FOOTER_STRING_MAX 512

typedef struct {
    sd_animation *animation;
    
    uint16_t unknown_0;
    uint16_t unknown_2;
    uint8_t unknown_3;
    uint8_t unknown_4;
    uint8_t unknown_5;
    uint8_t unknown_6;
    uint8_t unknown_7;
    uint8_t unknown_8;
    uint8_t unknown_9;
    uint8_t unknown_10;
    uint8_t unknown_11;
    uint8_t next_anim_id;
    uint8_t category;
    uint8_t unknown_14;
    uint8_t scrap_amount;
    uint8_t successor_id;
    uint8_t damage_amount;
    uint8_t unknown_18;
    uint8_t unknown_19;
    uint8_t points;

    char move_string[SD_MOVE_STRING_MAX];
    char footer_string[SD_MOVE_FOOTER_STRING_MAX];
} sd_move;

int sd_move_create(sd_move *move);
int sd_move_copy(sd_move *dst, const sd_move *src);
void sd_move_free(sd_move *move);

int sd_move_set_animation(sd_move *move, const sd_animation *animation);
sd_animation* sd_move_get_animation(const sd_move *move);
int sd_move_set_footer_string(sd_move *move, const char *str);
int sd_move_set_move_string(sd_move *move, const char *str);

#ifdef SD_USE_INTERNAL
int sd_move_load(sd_reader *reader, sd_move *move);
int sd_move_save(sd_writer *writer, const sd_move *move);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_MOVE_H
