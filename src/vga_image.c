#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "shadowdive/vga_image.h"
#include "shadowdive/error.h"

int sd_vga_image_create(sd_vga_image *img, unsigned int w, unsigned int h) {
    if(img == NULL) {
        return SD_INVALID_INPUT;
    }
    img->w = w;
    img->h = h;
    img->len = w * h;
    if((img->data = malloc(w * h)) == NULL) {
        goto error_0;
    }
    if((img->stencil = malloc(w * h)) == NULL) {
        goto error_1;
    }
    memset(img->data, 0, w * h);
    memset(img->stencil, 1, w * h);
    return SD_SUCCESS;

error_1:
    free(img->data);
error_0:
    return SD_OUT_OF_MEMORY;
}

int sd_vga_image_copy(sd_vga_image *dst, const sd_vga_image *src) {
    if(dst == NULL || src == NULL) {
        return SD_INVALID_INPUT;
    }
    dst->w = src->w;
    dst->h = src->h;
    dst->len = src->len;
    if((dst->data = malloc(src->len)) == NULL) {
        goto error_0;
    }
    if((dst->stencil = malloc(src->len)) == NULL) {
        goto error_1;
    }
    memcpy(dst->data, src->data, src->len);
    memcpy(dst->stencil, src->stencil, src->len);
    return SD_SUCCESS;

error_1:
    free(dst->data);
error_0:
    return SD_OUT_OF_MEMORY;
}

void sd_vga_image_free(sd_vga_image *img) {
    if(img == NULL) return;
    free(img->data);
    free(img->stencil);
}

int sd_vga_image_encode(sd_vga_image *dst, const sd_rgba_image *src, const sd_palette *pal, int remapping) {
    int ret;
    if(dst == NULL || src == NULL || pal == NULL) {
        return SD_INVALID_INPUT;
    }
    if((ret = sd_vga_image_create(dst, src->w, src->h)) != SD_SUCCESS) {
        return ret;
    }
    unsigned int rgb_size = (src->w * src->h * 4);
    for(int pos = 0; pos <= rgb_size; pos+= 4) {
        uint8_t r = src->data[pos];
        uint8_t g = src->data[pos+1];
        uint8_t b = src->data[pos+2];
        // ignore alpha channel, VGA images have no transparency
        dst->data[pos/4] = sd_palette_resolve_color(r, g, b, pal);;
    }
    return SD_SUCCESS;
}

int sd_vga_image_decode(sd_rgba_image *dst, const sd_vga_image *src, const sd_palette *pal, int remapping) {
    int ret;
    if(dst == NULL || src == NULL || pal == NULL) {
        return SD_INVALID_INPUT;
    }
    if((ret = sd_rgba_image_create(dst, src->w, src->h)) != SD_SUCCESS) {
        return ret;
    }
    int pos = 0;
    for(int y = src->h - 1; y >= 0; y--) {
        for(int x = 0; x < src->w; x++) {
            uint8_t b = src->data[y * src->w + x];
            uint8_t s = src->stencil[y * src->w + x];
            pos = ((y * src->w) + x) * 4;
            if(remapping > -1) {
                dst->data[pos+0] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][0];
                dst->data[pos+1] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][1];
                dst->data[pos+2] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][2];
            } else {
                dst->data[pos+0] = (uint8_t)pal->data[b][0];
                dst->data[pos+1] = (uint8_t)pal->data[b][1];
                dst->data[pos+2] = (uint8_t)pal->data[b][2];
            }
            // check the stencil to see if this is a real pixel
            if(s == 1) {
                dst->data[pos+3] = (uint8_t)255; // fully opaque
            } else {
                dst->data[pos+3] = (uint8_t)0; // fully transparent
            }
        }
    }
    return SD_SUCCESS;
}
