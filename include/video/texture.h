#ifndef _TEXTURE_H
#define _TEXTURE_H

typedef struct sd_rgba_image_t sd_rgba_image;

typedef struct texture_t {
    unsigned int id;
    sd_rgba_image *original;
} texture;

void texture_create(texture *tex, sd_rgba_image *img);
void texture_free(texture *tex);
void texture_validate(texture *tex);

#endif // _TEXTURE_H