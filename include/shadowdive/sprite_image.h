#ifndef _SD_SPRITE_IMAGE
#define _SD_SPRITE_IMAGE

#ifndef _SD_PALETTE_H
typedef struct sd_palette_t sd_palette;
#endif

#ifndef _SD_RGBA_IMAGE_H
typedef struct sd_rgba_image_t sd_rgba_image;
#endif

typedef struct sd_sprite_image_t {
    unsigned int w;
    unsigned int h;
    unsigned int len;
    char *data;
} sd_sprite_image;

sd_sprite_image* sd_sprite_image_create(unsigned int w, unsigned int h, unsigned int len);
void sd_sprite_image_delete(sd_sprite_image *img, int missing);
sd_sprite_image* sd_sprite_image_encode(sd_rgba_image *img, sd_palette *pal, int remapping);
sd_rgba_image* sd_sprite_image_decode(sd_sprite_image *img, sd_palette *pal, int remapping);

#endif // _SD_SPRITE_IMAGE
