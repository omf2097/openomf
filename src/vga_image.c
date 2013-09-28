#include "shadowdive/rgba_image.h"
#include "shadowdive/palette.h"
#include "shadowdive/vga_image.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

sd_vga_image* sd_vga_image_create(unsigned int w, unsigned int h) {
    sd_vga_image *img = (sd_vga_image*)malloc(sizeof(sd_vga_image));
    img->w = w;
    img->h = h;
    img->len = w*h;
    img->data = (char*)malloc(w*h);
    img->stencil = (char*)malloc(w*h);
    memset(img->data, 0, w*h);
    memset(img->stencil, 1, w*h);
    return img;
}

sd_vga_image* sd_vga_image_clone(sd_vga_image *src) {
    sd_vga_image *dest = (sd_vga_image*)malloc(sizeof(sd_vga_image));
    dest->w = src->w;
    dest->h = src->h;
    dest->len = src->len;
    dest->data = (char*)malloc(dest->len);
    dest->stencil = (char*)malloc(dest->len);
    memcpy(dest->data, src->data, src->len);
    memcpy(dest->stencil, src->stencil, src->len);
    return dest;
}

void sd_vga_image_delete(sd_vga_image *img) {
    free(img->data);
    free(img->stencil);
    free(img);
}

sd_vga_image* sd_vga_image_encode(sd_rgba_image *img, sd_palette *pal, int remapping) {
    sd_vga_image *vga = sd_vga_image_create(img->w, img->h);
    unsigned int rgb_size = (img->w * img->h * 4);
    unsigned char palette_index;

    for(int pos = 0; pos <= rgb_size; pos+= 4) {
        uint8_t r = img->data[pos];
        uint8_t g = img->data[pos+1];
        uint8_t b = img->data[pos+2];
        // ignore alpha channel, VGA images have no transparency
        palette_index = sd_palette_resolve_color(r, g, b, pal);
        vga->data[pos/4] = palette_index;
    }

    return vga;
}

sd_rgba_image* sd_vga_image_decode(sd_vga_image *img, sd_palette *pal, int remapping) {
    sd_rgba_image *rgba = sd_rgba_image_create(img->w, img->h);
    int pos = 0;
    for(int y = img->h - 1; y >= 0; y--) {
        for(int x = 0; x < img->w; x++) {
            uint8_t b = img->data[y * img->w + x];
            uint8_t s = img->stencil[y * img->w + x];
            pos = ((y * img->w) + x) * 4;
            if(remapping > -1) {
                rgba->data[pos+0] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][0];
                rgba->data[pos+1] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][1];
                rgba->data[pos+2] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][2];
            } else {
                rgba->data[pos+0] = (uint8_t)pal->data[b][0];
                rgba->data[pos+1] = (uint8_t)pal->data[b][1];
                rgba->data[pos+2] = (uint8_t)pal->data[b][2];
            }
            // check the stencil to see if this is a real pixel
            if (s == 1) {
                rgba->data[pos+3] = (uint8_t)255; // fully opaque
            } else {
                rgba->data[pos+3] = (uint8_t)0; // fully transparent
            }
        }
    }
    return rgba;
}
