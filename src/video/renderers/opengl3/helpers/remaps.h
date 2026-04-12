#ifndef REMAPS_H
#define REMAPS_H

#include "video/vga_remap.h"
#include <epoxy/gl.h>

typedef struct remaps remaps;

/**
 * Create a 1024x19 R16UI texture for palette remap tables.
 * Each row is a remap table mapping source palette index to destination index.
 *
 * @param texture_unit_id OpenGL texture unit to bind the remap texture to
 * @return Allocated remaps object, must be freed with remaps_free()
 */
remaps *remaps_create(GLuint texture_unit_id);

/**
 * Upload all remap tables to the texture.
 *
 * @param remaps Remaps object
 * @param data Source remap table data (1024x19 uint16 entries)
 */
void remaps_update(const remaps *remaps, const vga_remap_tables *data);

/**
 * Free the remap texture and object. Sets the pointer to NULL.
 *
 * @param maps Pointer to remaps object pointer
 */
void remaps_free(remaps **maps);

#endif // REMAPS_H
