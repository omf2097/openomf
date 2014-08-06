#ifndef _SD_SPRITE_H
#define _SD_SPRITE_H

#include <stdint.h>
#include "shadowdive/vga_image.h"
#include "shadowdive/rgba_image.h"
#ifdef SD_USE_INTERNAL
    #include "shadowdive/internal/reader.h"
    #include "shadowdive/internal/writer.h"
#endif

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct {
    int16_t pos_x;
    int16_t pos_y;
    uint8_t index;
    uint8_t missing;
    uint16_t width;
    uint16_t height;
    uint16_t len;
    char *data;
} sd_sprite;

int sd_sprite_create(sd_sprite *sprite);
int sd_sprite_copy(sd_sprite *dst, const sd_sprite *src);
void sd_sprite_free(sd_sprite *sprite);

int sd_sprite_rgba_encode(
    sd_sprite *dst,
    const sd_rgba_image *src,
    const sd_palette *pal,
    int remapping);

int sd_sprite_rgba_decode(
    sd_rgba_image *dst,
    const sd_sprite *src,
    const sd_palette *pal,
    int remapping);

int sd_sprite_vga_decode(
    sd_vga_image *dst,
    const sd_sprite *src);

#ifdef SD_USE_INTERNAL
int sd_sprite_load(sd_reader *reader, sd_sprite *sprite);
void sd_sprite_save(sd_writer *writer, const sd_sprite *sprite);
#endif

#ifdef __cplusplus
}
#endif

#endif // _SD_SPRITE_H
