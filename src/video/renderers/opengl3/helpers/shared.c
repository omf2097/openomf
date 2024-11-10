#include <stdalign.h>
#include <stdlib.h>

#include "utils/allocator.h"
#include "video/opengl/shared.h"
#include "video/opengl/ubo.h"

typedef struct data_buffer {
    alignas(16) GLfloat palette[1024]; // 256 * sizeof(vec4f)
} data_buffer;

typedef struct shared {
    GLuint ubo_id;
    data_buffer data;
} shared;

shared *shared_create(void) {
    shared *obj = omf_calloc(1, sizeof(shared));
    obj->ubo_id = ubo_create(sizeof(data_buffer));
    for(int i = 0; i < 1024; i++) {
        obj->data.palette[i] = 1.0f;
    }
    return obj;
}

void shared_set_palette(shared *shared, vga_palette *data, vga_index start, vga_index end) {
    GLsizeiptr items = end - start + 1;
    for(int i = 0; i < items; i++) {
        shared->data.palette[i * 4 + 0] = data->colors[start + i].r / 255.0f;
        shared->data.palette[i * 4 + 1] = data->colors[start + i].g / 255.0f;
        shared->data.palette[i * 4 + 2] = data->colors[start + i].b / 255.0f;
    }
    GLsizeiptr offset = start * sizeof(GLfloat) * 4;
    GLsizeiptr size = items * sizeof(GLfloat) * 4;
    ubo_update(shared->ubo_id, offset, size, &shared->data);
}

GLuint shared_get_block(shared *buffer) {
    return buffer->ubo_id;
}

void shared_free(shared **buffer) {
    shared *obj = *buffer;
    if(obj != NULL) {
        ubo_free(obj->ubo_id);
        omf_free(obj);
        *buffer = NULL;
    }
}
