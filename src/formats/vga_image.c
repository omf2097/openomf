#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats/error.h"
#include "formats/vga_image.h"
#include "utils/allocator.h"
#include "utils/png_reader.h"
#include "utils/png_writer.h"

int sd_vga_image_create(sd_vga_image *img, unsigned int w, unsigned int h) {
    if(img == NULL) {
        return SD_INVALID_INPUT;
    }
    img->w = w;
    img->h = h;
    img->len = w * h;
    img->data = omf_calloc(1, w * h);
    return SD_SUCCESS;
}

int sd_vga_image_copy(sd_vga_image *dst, const sd_vga_image *src) {
    if(dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }
    dst->w = src->w;
    dst->h = src->h;
    dst->len = src->len;
    dst->data = omf_calloc(src->len, 1);
    memcpy(dst->data, src->data, src->len);
    return SD_SUCCESS;
}

void sd_vga_image_free(sd_vga_image *img) {
    if(img == NULL) {
        return;
    }
    omf_free(img->data);
}

int sd_vga_image_decode(sd_rgba_image *dst, const sd_vga_image *src, const vga_palette *pal) {
    int ret;
    if(dst == NULL || src == NULL || pal == NULL) {
        return SD_INVALID_INPUT;
    }
    if((ret = sd_rgba_image_create(dst, src->w, src->h)) != SD_SUCCESS) {
        return ret;
    }
    int pos = 0;
    for(int y = src->h - 1; y >= 0; y--) {
        for(unsigned x = 0; x < src->w; x++) {
            uint8_t b = src->data[y * src->w + x];
            pos = ((y * src->w) + x) * 4;
            dst->data[pos + 0] = (uint8_t)pal->colors[b].r;
            dst->data[pos + 1] = (uint8_t)pal->colors[b].g;
            dst->data[pos + 2] = (uint8_t)pal->colors[b].b;
            dst->data[pos + 3] = (uint8_t)255;
        }
    }
    return SD_SUCCESS;
}

int sd_vga_image_from_png(sd_vga_image *img, const char *filename) {
    if(sd_vga_image_create(img, 320, 200) != SD_SUCCESS) {
        return SD_FAILURE;
    }
    if(!read_paletted_png(filename, (unsigned char *)img->data)) {
        return SD_FAILURE;
    }
    return SD_SUCCESS;
}

int sd_vga_image_to_png(const sd_vga_image *img, const vga_palette *pal, const char *filename) {
    if(img == NULL || filename == NULL) {
        return SD_INVALID_INPUT;
    }
    if(!write_paletted_png(filename, img->w, img->h, pal, (unsigned char *)img->data)) {
        return SD_FILE_OPEN_ERROR;
    }
    return SD_SUCCESS;
}
