#include <stdlib.h>

#include "utils/allocator.h"
#include "video/opengl/shared.h"
#include "video/opengl/ubo.h"

typedef struct data_buffer {
    GLfloat palette[1024]; // 256 * sizeof(vec4f)
} __attribute__((aligned(16))) data_buffer;

typedef struct shared {
    GLuint ubo_id;
    data_buffer data;
    bool dirty;
} shared;

shared *shared_create(void) {
    shared *obj = omf_calloc(1, sizeof(shared));
    obj->ubo_id = ubo_create(sizeof(data_buffer));
    obj->dirty = false;
    for(int i = 0; i < 1024; i++) {
        obj->data.palette[i] = 1.0f;
    }
    return obj;
}

void shared_set_palette(shared *shared, const void *data) {
    int w = 0;
    for(int i = 0; i < 256; i++) {
        unsigned char *ptr = (unsigned char *)data + i * 3;
        shared->data.palette[w++] = ((float)*(ptr + 0)) / 255.0f;
        shared->data.palette[w++] = ((float)*(ptr + 1)) / 255.0f;
        shared->data.palette[w++] = ((float)*(ptr + 2)) / 255.0f;
        w++;
    }
    shared->dirty = true;
}

void shared_flush_dirty(shared *shared) {
    if(shared->dirty) {
        ubo_update(shared->ubo_id, sizeof(data_buffer), &shared->data);
        shared->dirty = false;
    }
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
