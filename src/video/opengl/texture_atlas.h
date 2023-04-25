#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include <epoxy/gl.h>
#include <stdint.h>

typedef struct texture_atlas texture_atlas;

texture_atlas *atlas_create(uint16_t width, uint16_t height);
void atlas_free(texture_atlas **atlas);

void atlas_insert(texture_atlas *atlas, uint64_t id, const char *bytes, uint16_t width, uint16_t height);
void atlas_clean(texture_atlas *atlas);
GLuint atlas_get_texture_id(texture_atlas *atlas);

#endif // TEXTURE_ATLAS_H
