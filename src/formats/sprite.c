#include <stdlib.h>
#include <string.h>

#include "formats/error.h"
#include "formats/palette.h"
#include "formats/sprite.h"
#include "utils/allocator.h"
#include <stdio.h>

int sd_sprite_create(sd_sprite *sprite) {
    if (sprite == NULL) {
        return SD_INVALID_INPUT;
    }
    memset(sprite, 0, sizeof(sd_sprite));
    return SD_SUCCESS;
}

int sd_sprite_copy(sd_sprite *dst, const sd_sprite *src) {
    if (dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }

    // Clear destination
    memset(dst, 0, sizeof(sd_sprite));

    dst->pos_x = src->pos_x;
    dst->pos_y = src->pos_y;
    dst->index = src->index;
    dst->missing = src->missing;
    dst->width = src->width;
    dst->height = src->height;
    dst->len = src->len;

    if (src->data != NULL) {
        dst->data = omf_calloc(src->len, 1);
        memcpy(dst->data, src->data, src->len);
    }

    return SD_SUCCESS;
}

void sd_sprite_free(sd_sprite *sprite) {
    if (sprite == NULL)
        return;

    // Only attempt to free if there IS something to free
    // AND sprite data belongs to this sprite
    if (sprite->data != NULL && !sprite->missing) {
        omf_free(sprite->data);
    }
}

int sd_sprite_load(sd_reader *r, sd_sprite *sprite) {
    sprite->len = sd_read_uword(r);
    sprite->pos_x = sd_read_word(r);
    sprite->pos_y = sd_read_word(r);
    sprite->width = sd_read_uword(r);
    sprite->height = sd_read_uword(r);
    sprite->index = sd_read_ubyte(r);
    sprite->missing = sd_read_ubyte(r);

    // Copy sprite data, if there is any.
    if (sprite->missing == 0) {
        sprite->data = omf_calloc(sprite->len, 1);
        sd_read_buf(r, sprite->data, sprite->len);
    } else {
        sprite->data = NULL;
    }

    if (!sd_reader_ok(r)) {
        return SD_FILE_PARSE_ERROR;
    }
    return SD_SUCCESS;
}

int sd_sprite_save(sd_writer *w, const sd_sprite *sprite) {
    if (w == NULL || sprite == NULL) {
        return SD_INVALID_INPUT;
    }
    sd_write_uword(w, sprite->len);
    sd_write_word(w, sprite->pos_x);
    sd_write_word(w, sprite->pos_y);
    sd_write_uword(w, sprite->width);
    sd_write_uword(w, sprite->height);
    sd_write_ubyte(w, sprite->index);
    sd_write_ubyte(w, sprite->missing);
    if (!sprite->missing) {
        sd_write_buf(w, sprite->data, sprite->len);
    }
    return SD_SUCCESS;
}

int sd_sprite_rgba_encode(sd_sprite *dst, const sd_rgba_image *src, const palette *pal,
                          int remapping) {
    int lastx = -1;
    int lasty = 0;
    int i = 0;
    int rowlen = 0;
    uint16_t c = 0;
    int rowstart = 0;
    int ret = SD_SUCCESS;
    size_t rgb_size;
    char *buf;

    // Make sure we aren't being fed BS
    if (dst == NULL || src == NULL || pal == NULL) {
        return SD_INVALID_INPUT;
    }

    // allocate a buffer plenty big enough, we will trim it later
    rgb_size = src->w * src->h * 4;
    buf = omf_calloc(rgb_size, 1);

    // always initialize Y to 0
    buf[i++] = 2;
    buf[i++] = 0;
    rowstart = i;

    // Walk through the RGBA data
    for (int pos = 0; pos <= rgb_size; pos += 4) {
        uint8_t r = src->data[pos];
        uint8_t g = src->data[pos + 1];
        uint8_t b = src->data[pos + 2];
        uint8_t a = src->data[pos + 3];

        // ignore anytjhing but fully opaque pixels
        if (a == 255) {
            int16_t x = (pos / 4) % src->w;
            int16_t y = (pos / 4) / src->w;
            if (y != lasty) {
                // new row
                c = (y * 4) + 2;
                // write little endian unsigned word
                buf[i++] = c & 0x00ff;
                buf[i++] = (c & 0xff00) >> 8;
            }
            if (x != lastx + 1 || y != lasty) {
                // if Y changes, write out X too
                // dont write X coordinate if we just wrote a row and the nex X coordinate is 0
                // because the decoder resets X coordinate to 0 after each row
                if (x != 0) {
                    // we skipped some columns
                    c = (x * 4);
                    // write little endian unsigned word
                    buf[i++] = c & 0x00ff;
                    buf[i++] = (c & 0xff00) >> 8;
                }
                if (!rowlen) {
                    rowstart = i;
                    i += 2;
                }
            }
            // write out the length of the previous row, if there was one
            if (y != lasty || x != lastx + 1) {
                if (rowlen) {
                    // go back and write in the width of the row of pixels
                    c = (rowlen * 4) + 1;
                    // place to write is at i - rowlen
                    // write little endian unsigned word
                    buf[rowstart] = c & 0x00ff;
                    buf[rowstart + 1] = (c & 0xff00) >> 8;
                    rowlen = 0;
                    rowstart = i;
                    i += 2;
                }
            } else if (lasty == 0 && x == 0) {
                rowstart = i;
                i += 2;
            }
            lastx = x;
            lasty = y;
            // write byte
            buf[i++] = palette_resolve_color(r, g, b, pal);
            rowlen++;
        }
    }
    // update the length of the last row
    if (rowlen) {
        // go back and write in the width of the row of pixels
        c = (rowlen * 4) + 1;
        // place to write is at i - rowlen
        // write little endian unsigned word
        buf[rowstart] = c & 0x00ff;
        buf[rowstart + 1] = (c & 0xff00) >> 8;
    }

    // End of sprite marker, a WORD of value 7
    buf[i++] = 7;
    buf[i++] = 0;

    // Copy data
    dst->width = src->w;
    dst->height = src->h;
    dst->len = i;
    dst->missing = 0;
    dst->data = omf_calloc(i, 1);
    memcpy(dst->data, buf, i);
    omf_free(buf);
    return ret;
}

int sd_sprite_rgba_decode(sd_rgba_image *dst, const sd_sprite *src, const palette *pal,
                          int remapping) {
    uint16_t x = 0;
    uint16_t y = 0;
    int i = 0;
    uint16_t c = 0;
    uint16_t data = 0;
    char op = 0;

    // Make sure we aren't being fed BS
    if (src == NULL || dst == NULL || pal == NULL) {
        return SD_INVALID_INPUT;
    }

    // If image data length is 0, then size should be 1x1
    if (src->len > 0) {
        sd_rgba_image_create(dst, src->width, src->height);
    } else {
        sd_rgba_image_create(dst, 1, 1);
    }

    // XXX CREDITS.BK has a bunch of 0 width sprites, for some unknown reason
    if (src->width == 0 || src->height == 0 || src->len == 0) {
        return SD_SUCCESS;
    }

    // Walk through sprite raw data
    while (i < src->len) {
        // read a word
        c = (uint8_t)src->data[i] + ((uint8_t)src->data[i + 1] << 8);
        op = c % 4;
        data = c / 4;
        i += 2; // we read 2 bytes

        // Handle operation
        switch (op) {
        case 0:
            x = data;
            break;
        case 2:
            y = data;
            break;
        case 1:
            while (data > 0) {
                uint8_t b = src->data[i];
                int pos = ((y * src->width) + x) * 4;
                if (remapping > -1) {
                    dst->data[pos + 0] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][0];
                    dst->data[pos + 1] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][1];
                    dst->data[pos + 2] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][2];
                } else {
                    dst->data[pos + 0] = (uint8_t)pal->data[b][0];
                    dst->data[pos + 1] = (uint8_t)pal->data[b][1];
                    dst->data[pos + 2] = (uint8_t)pal->data[b][2];
                }
                dst->data[pos + 3] = (uint8_t)255; // fully opaque
                i++;                               // we read 1 byte
                x++;
                data--;
            }
            x = 0;
            break;
        case 3:
            if (i != src->len) {
                return SD_INVALID_INPUT;
            }
            break;
        }
    }

    // All done. dst should now contain a valid RGBA image.
    return SD_SUCCESS;
}

int sd_sprite_vga_decode(sd_vga_image *dst, const sd_sprite *src) {
    uint16_t x = 0;
    uint16_t y = 0;
    int i = 0;
    uint16_t c = 0;
    uint16_t data = 0;
    char op = 0;

    // Make sure we aren't being fed BS
    if (dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }

    // If image data length is 0, then size should be 1x1
    if (src->len > 0) {
        sd_vga_image_create(dst, src->width, src->height);
    } else {
        sd_vga_image_create(dst, 1, 1);
    }

    // XXX CREDITS.BK has a bunch of 0 width sprites, for some unknown reason
    if (src->width == 0 || src->height == 0 || src->len == 0) {
        return SD_SUCCESS;
    }

    // everything defaults to transparent
    int bsize = src->width * src->height;
    memset(dst->stencil, 0, bsize);

    // Walk through raw sprite data
    while (i < src->len) {
        // read a word
        c = (uint8_t)src->data[i] + ((uint8_t)src->data[i + 1] << 8);
        op = c % 4;
        data = c / 4;
        i += 2; // we read 2 bytes

        // Handle operation
        switch (op) {
        case 0:
            x = data;
            break;
        case 2:
            y = data;
            break;
        case 1:
            while (data > 0) {
                uint8_t b = src->data[i];
                int pos = ((y * src->width) + x);
                dst->data[pos] = b;
                dst->stencil[pos] = 1;
                i++; // we read 1 byte
                x++;
                data--;
            }
            x = 0;
            break;
        case 3:
            if (i != src->len) {
                return SD_INVALID_INPUT;
            }
            break;
        }
    }

    // All done. dst should now contain a valid vga image.
    return SD_SUCCESS;
}

int sd_sprite_vga_encode(sd_sprite *dst, const sd_vga_image *src) {
    int lastx = -1;
    int lasty = 0;
    int i = 0;
    int rowlen = 0;
    uint16_t c = 0;
    int rowstart = 0;
    int ret = SD_SUCCESS;
    size_t vga_size;
    char *buf;

    // Make sure we aren't being fed BS
    if (dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }

    // allocate a buffer plenty big enough, we will trim it later
    vga_size = src->w * src->h;
    buf = omf_calloc(4, vga_size);

    // always initialize Y to 0
    buf[i++] = 2;
    buf[i++] = 0;
    rowstart = i;

    // Walk through the RGBA data
    for (int pos = 0; pos < vga_size; pos++) {
        uint8_t idx = src->data[pos];
        uint8_t stc = src->stencil[pos];

        // ignore anything but fully opaque pixels
        if (stc == 1) {
            int16_t x = pos % src->w;
            int16_t y = pos / src->w;
            if (y != lasty) {
                c = (y * 4) + 2;
                buf[i++] = c & 0x00ff;
                buf[i++] = (c & 0xff00) >> 8;
            }
            if (x != lastx + 1 || y != lasty) {
                if (x != 0) {
                    c = (x * 4);
                    buf[i++] = c & 0x00ff;
                    buf[i++] = (c & 0xff00) >> 8;
                }
                if (!rowlen) {
                    rowstart = i;
                    i += 2;
                }
            }
            if (y != lasty || x != lastx + 1) {
                if (rowlen) {
                    c = (rowlen * 4) + 1;
                    buf[rowstart] = c & 0x00ff;
                    buf[rowstart + 1] = (c & 0xff00) >> 8;
                    rowlen = 0;
                    rowstart = i;
                    i += 2;
                }
            } else if (lasty == 0 && x == 0) {
                rowstart = i;
                i += 2;
            }
            lastx = x;
            lasty = y;
            buf[i++] = idx;
            rowlen++;
        }
    }
    if (rowlen) {
        c = (rowlen * 4) + 1;
        buf[rowstart] = c & 0x00ff;
        buf[rowstart + 1] = (c & 0xff00) >> 8;
    }

    // End of sprite marker, a WORD of value 7
    buf[i++] = 7;
    buf[i++] = 0;

    // Copy data
    dst->width = src->w;
    dst->height = src->h;
    dst->len = i;
    dst->missing = 0;
    dst->data = omf_calloc(i, 1);
    memcpy(dst->data, buf, i);
    omf_free(buf);
    return ret;
}
