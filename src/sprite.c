#include "sprite.h"
#include "internal/reader.h"
#include "internal/writer.h"
#include <stdlib.h>

sd_sprite* sd_sprite_create() {
    sd_sprite *sprite = (sd_sprite*)malloc(sizeof(sd_sprite));
    sprite->img = NULL;
    return sprite;
}

void sd_sprite_delete(sd_sprite *sprite) {
    sd_sprite_image_delete(sprite->img);
    free(sprite);
}

int sd_sprite_load(sd_reader *r, sd_sprite *sprite) {
    uint16_t len = sd_read_uword(r);
    sprite->pos_x = sd_read_word(r);
    sprite->pos_y = sd_read_word(r);
    uint16_t width = sd_read_uword(r);
    uint16_t height = sd_read_uword(r);
    sprite->index = sd_read_ubyte(r);
    sprite->missing = sd_read_ubyte(r);
    if (sprite->missing == 0) {
        // sprite data follows
        sprite->img = sd_sprite_image_create(width, height, len);
        sd_read_buf(r, sprite->img->data, len);
    } else {
        // TODO set the pointer to be the actual sprite, from the other animation, maybe?
        sprite->img = NULL;
    }
    return sd_reader_ok(r);
}

void sd_sprite_save(sd_writer *writer, sd_sprite *sprite) {
    sd_write_uword(writer, sprite->img->len);
    sd_write_word(writer, sprite->pos_x);
    sd_write_word(writer, sprite->pos_y);
    sd_write_uword(writer, sprite->img->w);
    sd_write_uword(writer, sprite->img->h);
    sd_write_ubyte(writer, sprite->index);
    sd_write_ubyte(writer, sprite->missing);
    if(!sprite->missing) {
        sd_write_buf(writer, sprite->img->data, sprite->img->len);
    }
}
