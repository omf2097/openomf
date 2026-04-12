#ifndef GL_PALETTE_H
#define GL_PALETTE_H

#include "video/vga_palette.h"
#include <epoxy/gl.h>

typedef struct gl_palette gl_palette;

/**
 * Create a 1024x1 RGB8 texture for palette color storage.
 *
 * @param texture_unit_id OpenGL texture unit to bind the palette texture to
 * @return Allocated palette object, must be freed with gl_palette_free()
 */
gl_palette *gl_palette_create(GLuint texture_unit_id);

/**
 * Upload a range of palette colors to the texture.
 *
 * @param pal Palette object
 * @param data Source palette containing the color data
 * @param first First palette index to update (inclusive)
 * @param last Last palette index to update (inclusive)
 */
void gl_palette_update(const gl_palette *pal, const vga_palette *data, vga_index first, vga_index last);

/**
 * Free the palette texture and object. Sets the pointer to NULL.
 *
 * @param pal Pointer to palette object pointer
 */
void gl_palette_free(gl_palette **pal);

#endif // GL_PALETTE_H
