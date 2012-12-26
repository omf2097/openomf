#include "vga_image.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

vga_image* sd_vga_image_create(unsigned int w, unsigned int h) {
    vga_image *img = (vga_image*)malloc(sizeof(vga_image));
    img->w = w;
    img->h = h;
    img->len = w*h;
    img->data = (char*)malloc(w*h);
    memset(img->data, w*h, 0);
    return img;
}

void sd_vga_image_delete(vga_image *img) {
    free(img->data);
    free(img);
}

vga_image* sd_vga_image_encode(rgba_image *img, palette *pal, int remapping) {
    vga_image *vga = sd_vga_image_create(img->w, img->h);
    return vga;
}

rgba_image* sd_vga_image_decode(vga_image *img, palette *pal, int remapping) {
    rgba_image *rgba = sd_rgba_image_create(img->w, img->h);
    int pos = 0;
    for(int y = img->h - 1; y >= 0; y--) {
        for(int x = 0; x < img->w; x++) {
            uint8_t b = img->data[y * img->w + x];
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
            rgba->data[pos+3] = 0;
        }
    }
    return rgba;
}
