#include <epoxy/gl.h>
#include <stdlib.h>

#include "utils/allocator.h"
#include "utils/log.h"
#include "video/enums.h"
#include "video/opengl/buffers.h"
#include "video/opengl/object_array.h"

#define OBJ_SIZE 16
#define OBJ_BYTES (OBJ_SIZE * sizeof(GLfloat))
#define MAX_FANS 512
#define VBO_SIZE (MAX_FANS * OBJ_BYTES)
#define BUFFER_COUNT 2

typedef struct object_array {
    GLuint vbo_ids[BUFFER_COUNT];
    GLuint vao_id;
    int vbo_flip;
    int item_count;
    GLfloat *mapping;
    GLint fans_starts[MAX_FANS];
    GLsizei fans_sizes[MAX_FANS];
} object_array;

static void setup_vao_layout() {
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0); // NOLINT
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat))); // NOLINT
    glEnableVertexAttribArray(1);
}

object_array *object_array_create() {
    object_array *array = omf_calloc(1, sizeof(object_array));
    array->item_count = 0;
    array->mapping = NULL;
    for(int i = 0; i < BUFFER_COUNT; i++)
        array->vbo_ids[i] = vbo_create(VBO_SIZE);
    array->vao_id = vao_create();
    setup_vao_layout();
    return array;
}

void object_array_free(object_array **array) {
    object_array *obj = *array;
    if(obj != NULL) {
        vao_free(obj->vao_id);
        for(int i = 0; i < BUFFER_COUNT; i++)
            vbo_free(obj->vbo_ids[1]);
        omf_free(obj);
        *array = NULL;
    }
}

void object_array_prepare(object_array *array) {
    if(array->mapping != NULL) {
        PERROR("VBO is already mapped! Remember to call object_array_finish.");
        return;
    }
    array->vbo_flip = (array->vbo_flip + 1) % BUFFER_COUNT;
    array->mapping = vbo_map(array->vbo_ids[array->vbo_flip], VBO_SIZE);
    array->item_count = 0;
}

void object_array_finish(object_array *array) {
    vbo_unmap(array->vbo_ids[array->vbo_flip]);
    array->mapping = NULL;
}

void object_array_draw(const object_array *array) {
    glMultiDrawArrays(GL_TRIANGLE_FAN, array->fans_starts, array->fans_sizes, array->item_count);
}

#define COORDS(ptr, offset, cx, cy, tx, ty)                                                                            \
    ptr[offset + 0] = cx;                                                                                              \
    ptr[offset + 1] = cy;                                                                                              \
    ptr[offset + 2] = tx;                                                                                              \
    ptr[offset + 3] = ty;

void object_array_add(object_array *array, int x, int y, int w, int h, int tx, int ty, int tw, int th, int flags) {
    if(array->item_count >= MAX_FANS) {
        PERROR("Too many objects!");
        return;
    }
    float dx = 1.0f / 4096.0f;
    float dy = 1.0f / 4096.0f;

    float tx0, tx1;
    if(flags & FLIP_HORIZONTAL) {
        tx0 = (tx + tw) * dx;
        tx1 = tx * dx;
    } else {
        tx0 = tx * dx;
        tx1 = (tx + tw) * dx;
    }

    float ty0, ty1;
    if(flags & FLIP_VERTICAL) {
        ty0 = (ty + th) * dy;
        ty1 = ty * dy;
    } else {
        ty0 = ty * dy;
        ty1 = (ty + th) * dy;
    }

    GLfloat *coords = array->mapping + array->item_count * OBJ_SIZE;
    COORDS(coords, 0, x + w, y + h, tx1, ty1);
    COORDS(coords, 4, x, y + h, tx0, ty1);
    COORDS(coords, 8, x, y, tx0, ty0);
    COORDS(coords, 12, x + w, y, tx1, ty0);

    array->fans_starts[array->item_count] = array->item_count * 4;
    array->fans_sizes[array->item_count] = 4;
    array->item_count++;
}
