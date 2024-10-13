#include <stdlib.h>

#include "utils/allocator.h"
#include "video/opengl/buffers.h"
#include "video/opengl/remaps.h"

#define REMAPS_WIDTH 256
#define REMAPS_HEIGHT 19

typedef struct remaps {
    GLuint tex_unit;
    GLuint texture_id;
} remaps;

remaps *remaps_create(GLuint tex_unit) {
    remaps *maps = omf_calloc(1, sizeof(remaps));
    maps->tex_unit = tex_unit;
    maps->texture_id = texture_create(tex_unit, REMAPS_WIDTH, REMAPS_HEIGHT, GL_R8, GL_RED);
    return maps;
}

void remaps_update(const remaps *remaps, const char *data) {
    texture_update(remaps->tex_unit, remaps->texture_id, 0, 0, REMAPS_WIDTH, REMAPS_HEIGHT, GL_RED, data);
}

void remaps_free(remaps **maps) {
    remaps *obj = *maps;
    if(obj != NULL) {
        texture_free(obj->tex_unit, obj->texture_id);
        omf_free(obj);
    }
}
