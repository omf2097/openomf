#ifndef _SD_SPRITE_H
#define _SD_SPRITE_H

#ifdef USE_INTERNAL
#include "shadowdive/internal/reader.h"
#include "shadowdive/internal/writer.h"
#endif

#include <stdint.h>
#include "shadowdive/sprite_image.h"

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    int16_t pos_x;
    int16_t pos_y;
    uint8_t index;
    uint8_t missing;
    sd_sprite_image *img;
} sd_sprite;

int sd_sprite_create(sd_sprite *sprite);
int sd_sprite_copy(sd_sprite *sprite, const sd_sprite *src);
void sd_sprite_free(sd_sprite *sprite);

#ifdef USE_INTERNAL
int sd_sprite_load(sd_reader *reader, sd_sprite *sprite);
void sd_sprite_save(sd_writer *writer, sd_sprite *sprite);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_SPRITE_H
