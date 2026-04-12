#include "video/renderers/opengl3/helpers/palette.h"
#include "utils/allocator.h"
#include "video/renderers/opengl3/helpers/texture.h"

typedef struct gl_palette {
    GLuint texture_unit_id;
    GLuint texture_id;
} gl_palette;

gl_palette *gl_palette_create(GLuint texture_unit_id) {
    gl_palette *pal = omf_calloc(1, sizeof(gl_palette));
    pal->texture_unit_id = texture_unit_id;
    pal->texture_id =
        texture_create(texture_unit_id, VGA_PALETTE_SIZE, 1, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, GL_NEAREST);
    return pal;
}

void gl_palette_update(const gl_palette *pal, const vga_palette *data, const vga_index first, const vga_index last) {
    const GLsizei count = last - first + 1;
    texture_update(pal->texture_unit_id, pal->texture_id, first, 0, count, 1, GL_RGB, GL_UNSIGNED_BYTE,
                   (const char *)&data->colors[first]);
}

void gl_palette_free(gl_palette **pal) {
    gl_palette *obj = *pal;
    if(obj != NULL) {
        texture_free(obj->texture_unit_id, obj->texture_id);
        omf_free(obj);
        *pal = NULL;
    }
}
