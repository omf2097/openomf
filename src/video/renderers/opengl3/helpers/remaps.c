#include <stdlib.h>

#include "utils/allocator.h"
#include "video/renderers/opengl3/helpers/remaps.h"
#include "video/renderers/opengl3/helpers/texture.h"

#define REMAPS_WIDTH VGA_PALETTE_SIZE
#define REMAPS_HEIGHT 19

typedef struct remaps {
    GLuint texture_unit_id;
    GLuint texture_id;
} remaps;

remaps *remaps_create(const GLuint texture_unit_id) {
    remaps *maps = omf_calloc(1, sizeof(remaps));
    maps->texture_unit_id = texture_unit_id;
    GLenum internal_fmt = (sizeof(vga_pixel) == 2) ? GL_R16UI : GL_R8UI;
    GLenum type = (sizeof(vga_pixel) == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE;
    maps->texture_id =
        texture_create(texture_unit_id, REMAPS_WIDTH, REMAPS_HEIGHT, internal_fmt, GL_RED_INTEGER, type, GL_NEAREST);
    return maps;
}

void remaps_update(const remaps *remaps, const vga_remap_tables *data) {
    GLenum type = (sizeof(vga_pixel) == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_BYTE;
    texture_update(remaps->texture_unit_id, remaps->texture_id, 0, 0, REMAPS_WIDTH, REMAPS_HEIGHT, GL_RED_INTEGER, type,
                   data);
}

void remaps_free(remaps **maps) {
    remaps *obj = *maps;
    if(obj != NULL) {
        texture_free(obj->texture_unit_id, obj->texture_id);
        omf_free(obj);
        *maps = NULL;
    }
}
