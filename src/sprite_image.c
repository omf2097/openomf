#include "sprite_image.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

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
    while(i < img->len) {
        // read a word
        uint16_t c = (uint16_t)img->data[i] + ((uint16_t)img->data[i+1] << 8);
        char op = c % 4;
        uint16_t data = c / 4;
        i += 2; // we read 2 bytes
        switch(op) {
            case 0:
                printf("setting X to %d\n", data);
                x = data;
                break;
            case 2:
                printf("setting Y to %d\n", data);
                y = data;
                break;
            case 1:
                printf("reading %d bytes\n", data);
                while(data > 0) {
                    uint8_t b = img->data[y * img->w + x];
                    int pos = ((y * img->w) + x) * 4;
                    printf("b %d, pos%d -- %d -- %d x %d @ %d,%d\n", b, pos, rgba->w * rgba->h * 4, rgba->w, rgba->h, x, y);
                    if(remapping > -1) {
                        rgba->data[pos+0] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][0];
                        rgba->data[pos+1] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][1];
                        rgba->data[pos+2] = (uint8_t)pal->data[(uint8_t)pal->remaps[remapping][b]][2];
                    } else {
                        rgba->data[pos+0] = (uint8_t)pal->data[b][0];
                        rgba->data[pos+1] = (uint8_t)pal->data[b][1];
                        rgba->data[pos+2] = (uint8_t)pal->data[b][2];
                    }
                    i++; // we read 1 byte
                    x++;
                    data--;
                }
                printf("resetting X to 0\n");
                x = 0;
                break;
            case 3:
                return rgba;
                break;
        }
    }
    return rgba;
}

