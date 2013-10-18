#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "video/image.h"

typedef struct texture_t {
    unsigned int id;
    unsigned int w, h;
} texture;

void texture_create(texture *tex);
int texture_init(texture *tex, const char *data, unsigned int w, unsigned int h);
int texture_init_from_img(texture *tex, const image *img);
int texture_upload(texture *tex, const char* data);
void texture_free(texture *tex);
int texture_is_valid(texture *tex);
void texture_bind(texture *tex);
void texture_unbind();
unsigned int texture_size(texture *tex);

#endif // _TEXTURE_H
