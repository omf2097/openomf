#ifndef _SPRITE_IMAGE
#define _SPRITE_IMAGE

#include "rgba_image.h"

typedef struct sprite_image_t {
    unsigned int w;
    unsigned int h;
    unsigned int len;
    char *data;
} sprite_image;

sprite_image* sd_sprite_image_create(unsigned int w, unsigned int h);
void sd_sprite_image_delete(sprite_image *img);
sprite_image* sd_sprite_image_encode(rgba_image* img);
rgba_image* sd_sprite_image_decode(sprite_image *img);

#endif // _SPRITE_IMAGE
