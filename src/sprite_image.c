#include "sprite_image.h"
#include <stdlib.h>

sprite_image* sd_sprite_image_create(unsigned int w, unsigned int h) {
    return 0;
}

void sd_sprite_image_delete(sprite_image *img) {

}

sprite_image* sd_sprite_image_encode(rgba_image *img, palette *pal, int remapping) {
    sprite_image *sprite = sd_sprite_image_create(img->w, img->h);
    return sprite;
}

rgba_image* sd_sprite_image_decode(sprite_image *img, palette *pal, int remapping) {
    rgba_image *rgba = sd_rgba_image_create(img->w, img->h);
    return rgba;
}

