#include "sprite_image.h"
#include <stdlib.h>

sd_sprite_image* sd_sprite_image_create(unsigned int w, unsigned int h) {
    return 0;
}

void sd_sprite_image_delete(sd_sprite_image *img) {

}

sd_sprite_image* sd_sprite_image_encode(sd_rgba_image *img, sd_palette *pal, int remapping) {
    sd_sprite_image *sprite = sd_sprite_image_create(img->w, img->h);
    return sprite;
}

sd_rgba_image* sd_sprite_image_decode(sd_sprite_image *img, sd_palette *pal, int remapping) {
    sd_rgba_image *rgba = sd_rgba_image_create(img->w, img->h);
    return rgba;
}

