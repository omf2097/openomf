#ifndef _SPRITE_IMAGE
#define _SPRITE_IMAGE

#include "shadowdive/rgba_image.h"
#include "shadowdive/palette.h"

typedef struct sprite_image_t {
    unsigned int w;
    unsigned int h;
    unsigned int len;
    char *data;
} sd_sprite_image;

sd_sprite_image* sd_sprite_image_create(unsigned int w, unsigned int h, unsigned int len);
void sd_sprite_image_delete(sd_sprite_image *img);
sd_sprite_image* sd_sprite_image_encode(sd_rgba_image *img, sd_palette *pal, int remapping);
sd_rgba_image* sd_sprite_image_decode(sd_sprite_image *img, sd_palette *pal, int remapping);

#endif // _SPRITE_IMAGE
