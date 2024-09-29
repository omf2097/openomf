#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include "video/surface.h"
#include <epoxy/gl.h>
#include <stdint.h>

typedef struct texture_atlas texture_atlas;

texture_atlas *atlas_create(GLuint tex_unit, uint16_t width, uint16_t height);
void atlas_free(texture_atlas **atlas);

bool atlas_insert(texture_atlas *atlas, const char *bytes, uint16_t w, uint16_t h, uint16_t *nx, uint16_t *ny);
bool atlas_get(texture_atlas *atlas, const surface *surface, uint16_t *x, uint16_t *y, uint16_t *w, uint16_t *h);
void atlas_reset(texture_atlas *atlas);

#endif // TEXTURE_ATLAS_H
