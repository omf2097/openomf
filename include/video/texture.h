#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "video/image.h"

typedef struct texture_t {
    char *data;
    unsigned int id;
    unsigned int w, h;
} texture;

int texture_create(texture *tex, const char *data, unsigned int w, unsigned int h);
int texture_create_from_img(texture *tex, const image *img);
void texture_free(texture *tex);
int texture_valid(texture *tex);
int texture_revalidate(texture *tex);
int texture_pix_opaque(texture *tex, unsigned int x, unsigned int y);
void texture_bind(texture *tex);
void texture_unbind();
unsigned int texture_size(texture *tex);

#endif // _TEXTURE_H
