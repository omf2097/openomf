#ifndef _TEXTURE_H
#define _TEXTURE_H

typedef struct sd_rgba_image_t sd_rgba_image;

typedef struct texture_t {
    unsigned int id;
} texture;

void texture_create(texture *tex, const char *data, unsigned int w, unsigned int h);
void texture_free(texture *tex);
int texture_valid(texture *tex);
void texture_bind(texture *tex);
void texture_unbind();

#endif // _TEXTURE_H