#include "video/opengl/remaps.h"
#include "utils/allocator.h"
#include "video/opengl/buffers.h"

#define UBO_COUNT (256 * 19)
#define UBO_SIZE (UBO_COUNT * sizeof(GLfloat))

typedef struct remaps {
    GLuint ubo_id;
} remaps;

// TODO: Move the remappings to 1D array texture!

remaps *remaps_create() {
    remaps *maps = omf_calloc(1, sizeof(remaps));
    maps->ubo_id = ubo_create(UBO_SIZE * 4);
    return maps;
}

GLuint remaps_get_block_id(const remaps *remaps) {
    return remaps->ubo_id;
}

void remaps_update(remaps *remaps, const unsigned char *data) {
    GLfloat tmp[UBO_COUNT * 4];
    int ptr = 0;
    for(int i = 0; i < UBO_COUNT; i++) {
        float d = data[i] / 255.0f;
        tmp[ptr++] = d;
        tmp[ptr++] = d;
        tmp[ptr++] = d;
        tmp[ptr++] = d;
    }
    ubo_update(remaps->ubo_id, UBO_SIZE * 4, tmp);
}

void remaps_free(remaps **maps) {
    remaps *obj = *maps;
    if(obj != NULL) {
        ubo_free(obj->ubo_id);
        *maps = NULL;
    }
}