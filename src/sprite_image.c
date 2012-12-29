#include "sprite_image.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

sd_sprite_image* sd_sprite_image_create(unsigned int w, unsigned int h, unsigned int len) {
    sd_sprite_image *img = (sd_sprite_image*)malloc(sizeof(sd_sprite_image));
    img->w = w;
    img->h = h;
    img->len = len;
    img->data = (char*)malloc(len);
    return img;
}

void sd_sprite_image_delete(sd_sprite_image *img) {
    free(img->data);
    free(img);
}

sd_sprite_image* sd_sprite_image_encode(sd_rgba_image *img, sd_palette *pal, int remapping) {
    /*sd_sprite_image *sprite = sd_sprite_image_create(img->w, img->h);*/
    /*int lastx = -1;*/
    /*unsigned int rgb_size = (img->w * img->h * 4);*/
    /*for(int pos = 0; pos <= rgb_size; pos+= 4) {*/

    return 0;
}

sd_rgba_image* sd_sprite_image_decode(sd_sprite_image *img, sd_palette *pal, int remapping) {
    sd_rgba_image *rgba = sd_rgba_image_create(img->w, img->h);
    uint16_t x = 0;
    uint16_t y = 0;
    int i = 0;
    if (img->w == 0 || img->h == 0) {
        // XXX CREDITS.BK has a bunch of 0 width sprites, for some unknown reason
        return rgba;
    }
    while(i < img->len) {
        // read a word
        uint16_t c = (uint8_t)img->data[i] + ((uint8_t)img->data[i+1] << 8);
        /*printf("c is %d\n", c);*/
        char op = c % 4;
        uint16_t data = c / 4;
        i += 2; // we read 2 bytes
        switch(op) {
            case 0:
                x = data;
                break;
            case 2:
                y = data;
                break;
            case 1:
                while(data > 0) {
                    uint8_t b = img->data[i];
                    int pos = ((y * img->w) + x) * 4;
                    if(remapping > -1) {
                        rgba->data[pos+0] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][0];
                        rgba->data[pos+1] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][1];
                        rgba->data[pos+2] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][2];
                    } else {
                        rgba->data[pos+0] = (uint8_t)pal->data[b][0];
                        rgba->data[pos+1] = (uint8_t)pal->data[b][1];
                        rgba->data[pos+2] = (uint8_t)pal->data[b][2];
                    }
                    rgba->data[pos+3] = 255; // fully opaque
                    i++; // we read 1 byte
                    x++;
                    data--;
                }
                x = 0;
                break;
            case 3:
                assert(i == img->len);
                break;
        }
    }
    return rgba;
}

