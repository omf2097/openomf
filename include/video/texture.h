#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "video/image.h"

typedef struct texture_t {
    unsigned int id;
    unsigned int w, h;
} texture;

int texture_create(texture *tex, const char *data, unsigned int w, unsigned int h);
int texture_create_from_img(texture *tex, const image *img);
void texture_free(texture *tex);
int texture_is_valid(texture *tex);
void texture_bind(texture *tex);
void texture_unbind();
unsigned int texture_size(texture *tex);

#endif // _TEXTURE_H
