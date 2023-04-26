#include <epoxy/gl.h>
#include <stdlib.h>

#include "utils/allocator.h"
#include "utils/log.h"
#include "video/opengl/object_array.h"

#define VBO_SIZE 8192
#define MAX_FANS 256
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

static GLuint create_vbo() {
    GLuint vbo_id;
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, VBO_SIZE, NULL, GL_DYNAMIC_DRAW);
    return vbo_id;
}

static GLuint create_vao() {
    GLuint vao_id;
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
    glEnableVertexAttribArray(0);
    return vao_id;
}

object_array *object_array_create() {
    object_array *array = omf_calloc(1, sizeof(object_array));
    array->item_count = 0;
    array->mapping = NULL;
    for(int i = 0; i < BUFFER_COUNT; i++) {
        array->vbo_ids[i] = create_vbo();
    }
    array->vao_id = create_vao();
    glBindVertexArray(array->vao_id);
    return array;
}

void object_array_free(object_array **array) {
    object_array *obj = *array;
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &obj->vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(BUFFER_COUNT, obj->vbo_ids);
    glDeleteBuffers(1, &obj->vbo_ids[1]);
    omf_free(obj);
    *array = NULL;
}

void object_array_prepare(object_array *array) {
    if(array->mapping != NULL) {
        PERROR("VBO is already mapped! Remember to call object_array_finish.");
        return;
    }
    array->vbo_flip = (array->vbo_flip + 1) % BUFFER_COUNT;
    glBindBuffer(GL_ARRAY_BUFFER, array->vbo_ids[array->vbo_flip]);
    array->mapping = glMapBufferRange(GL_ARRAY_BUFFER, 0, VBO_SIZE, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    array->item_count = 0;
}

void object_array_finish(object_array *array) {
    glUnmapBuffer(GL_ARRAY_BUFFER);
    array->mapping = NULL;
}

void object_array_draw(const object_array *array) {
    glMultiDrawArrays(GL_TRIANGLE_FAN, array->fans_starts, array->fans_sizes, array->item_count);
}

void object_array_add(object_array *array, int x, int y, int w, int h) {
    if(array->item_count >= MAX_FANS) {
        PERROR("Too many objects!");
        return;
    }

    int i = 0;
    GLfloat *coords = array->mapping + array->item_count * 16;
    coords[i++] = x + w;
    coords[i++] = y + h;
    coords[i++] = 1.0f;
    coords[i++] = 1.0f;

    coords[i++] = x;
    coords[i++] = y + h;
    coords[i++] = -1.0f;
    coords[i++] = 1.0f;

    coords[i++] = x;
    coords[i++] = y;
    coords[i++] = -1.0f;
    coords[i++] = -1.0f;

    coords[i++] = x + w;
    coords[i++] = y;
    coords[i++] = 1.0f;
    coords[i++] = -1.0f;

    array->fans_starts[array->item_count] = array->item_count * 4;
    array->fans_sizes[array->item_count] = 4;
    array->item_count++;
}
