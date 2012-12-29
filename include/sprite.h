#ifndef _SPRITE_H
#define _SPRITE_H

#include <inttypes.h>
#include "sprite_image.h"

typedef struct sd_writer_t sd_writer;
typedef struct sd_reader_t sd_reader;

typedef struct sprite_t {
    int16_t pos_x;
    int16_t pos_y;
    uint8_t index;
    uint8_t missing;
    sd_sprite_image *img;
} sd_sprite;

sd_sprite* sd_sprite_create();
void sd_sprite_delete(sd_sprite *sprite);
int sd_sprite_load(sd_reader *reader, sd_sprite *sprite);
void sd_sprite_save(sd_writer *writer, sd_sprite *sprite);

#endif // _SPRITE_H
