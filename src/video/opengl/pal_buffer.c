#include <stdlib.h>

#include "utils/allocator.h"
#include "video/opengl/buffers.h"
#include "video/opengl/pal_buffer.h"

#define PAL_SIZE (1024 * sizeof(GLfloat))

typedef struct pal_buffer {
    GLuint ubo_id;
} pal_buffer;

pal_buffer *pal_buffer_create() {
    pal_buffer *obj = omf_calloc(1, sizeof(pal_buffer));
    obj->ubo_id = ubo_create(PAL_SIZE);
    return obj;
}

void pal_buffer_update(pal_buffer *buffer, const void *data) {
    GLfloat arr[1024];
    int w = 0;
    for(int i = 0; i < 256; i++) {
        unsigned char *ptr = (unsigned char *)data + i * 3;
        arr[w++] = ((float)*(ptr + 0)) / 255.0f;
        arr[w++] = ((float)*(ptr + 1)) / 255.0f;
        arr[w++] = ((float)*(ptr + 2)) / 255.0f;
        arr[w++] = 1.0f;
    }
    ubo_update(buffer->ubo_id, PAL_SIZE, arr);
}

GLuint pal_buffer_get_block(pal_buffer *buffer) {
    return buffer->ubo_id;
}

void pal_buffer_free(pal_buffer **buffer) {
    pal_buffer *obj = *buffer;
    if(obj != NULL) {
        ubo_free(obj->ubo_id);
        omf_free(obj);
        *buffer = NULL;
    }
}
