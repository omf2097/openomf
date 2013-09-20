#ifndef _SD_SPRITE_H
#define _SD_SPRITE_H

#include <stdint.h>
#include "shadowdive/sprite_image.h"

#ifdef __cplusplus 
extern "C" {
#endif

#ifndef _SD_WRITER_H
typedef struct sd_writer_t sd_writer;
#endif

#ifndef _SD_READER_H
typedef struct sd_reader_t sd_reader;
#endif

typedef struct sd_sprite_t {
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

#ifdef __cplusplus
}
#endif

#endif // _SD_SPRITE_H
