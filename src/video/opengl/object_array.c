#include <epoxy/gl.h>
#include <stdlib.h>

#include "utils/allocator.h"
#include "utils/log.h"
#include "video/enums.h"
#include "video/opengl/buffers.h"
#include "video/opengl/object_array.h"

#define OBJ_BYTES (sizeof(object_data) * 4)
#define MAX_FANS 2048
#define VBO_SIZE (MAX_FANS * OBJ_BYTES)
#define BUFFER_COUNT 2

typedef struct {
    GLfloat x;
    GLfloat y;
    GLfloat tex_x;
    GLfloat tex_y;
    GLint transparency;
    GLint blend_mode;
    GLint palette_offset;
    GLint palette_limit;
} __attribute__((aligned(4))) object_data;

typedef struct object_array {
    GLuint vbo_ids[BUFFER_COUNT];
    GLuint vao_id;
    GLfloat src_w; // Source texture width
    GLfloat src_h; // Source texture height
    int vbo_flip;
    int item_count;
    object_data *mapping;
    GLint fans_starts[MAX_FANS];
    GLsizei fans_sizes[MAX_FANS];
    int modes[MAX_FANS];
    int mode_flip;
} object_array;

#define ATTRIB(index, stride, step, size, type, normalize)                                                             \
    glVertexAttribPointer(index, size, type, normalize, stride, step);                                                 \
    glEnableVertexAttribArray(index);                                                                                  \
    step += size * sizeof(GLfloat);                                                                                    \
    index++

#define ATTRIB_I(index, stride, step, size, type)                                                                      \
    glVertexAttribIPointer(index, size, type, stride, step);                                                           \
    glEnableVertexAttribArray(index);                                                                                  \
    step += size * sizeof(GLint);                                                                                      \
    index++

static void setup_vao_layout(void) {
    int stride = 4 * sizeof(GLfloat) + 4 * sizeof(GLint);
    int index = 0;
    unsigned char *step = 0;
    ATTRIB(index, stride, step, 2, GL_FLOAT, GL_FALSE);
    ATTRIB(index, stride, step, 2, GL_FLOAT, GL_FALSE);
    ATTRIB_I(index, stride, step, 1, GL_INT);
    ATTRIB_I(index, stride, step, 1, GL_INT);
    ATTRIB_I(index, stride, step, 1, GL_INT);
    ATTRIB_I(index, stride, step, 1, GL_INT);
}

object_array *object_array_create(GLfloat src_w, GLfloat src_h) {
    object_array *array = omf_calloc(1, sizeof(object_array));
    array->item_count = 0;
    array->mapping = NULL;
    array->src_w = src_w;
    array->src_h = src_h;
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
    array->mode_flip = -1;
    array->vbo_flip = (array->vbo_flip + 1) % BUFFER_COUNT;
    array->mapping = vbo_map(array->vbo_ids[array->vbo_flip], VBO_SIZE);
    array->item_count = 0;
}

void object_array_finish(object_array *array) {
    vbo_unmap(array->vbo_ids[array->vbo_flip]);
    array->mapping = NULL;
}

void object_array_begin(const object_array *array, object_array_batch *state) {
    state->start = 0;
    state->end = 0;
    state->mode = (array->item_count > 0) ? array->modes[0] : 0;
}

bool object_array_get_batch(const object_array *array, object_array_batch *state, video_blend_mode *mode) {
    if(state->end >= array->item_count) {
        return false;
    }
    state->start = state->end;
    video_blend_mode next;
    for(; state->end < array->item_count; state->end++) {
        next = array->modes[state->end];
        if(next != state->mode)
            break;
    }
    *mode = state->mode;
    state->mode = next;
    return true;
}

void object_array_draw(const object_array *array, object_array_batch *state) {
    int count = state->end - state->start;
    glMultiDrawArrays(GL_TRIANGLE_FAN, array->fans_starts + state->start, array->fans_sizes + state->start, count);
}

#define COORDS(ptr, cx, cy, tx, ty, transparency, mode, pal_offset, pal_limit)                                         \
    ptr.x = cx;                                                                                                        \
    ptr.y = cy;                                                                                                        \
    ptr.tex_x = tx;                                                                                                    \
    ptr.tex_y = ty;                                                                                                    \
    ptr.transparency = transparency;                                                                                   \
    ptr.blend_mode = mode;                                                                                             \
    ptr.palette_offset = pal_offset;                                                                                   \
    ptr.palette_limit = pal_limit;

static void add_item(object_array *array, float dx, float dy, int x, int y, int w, int h, int tx, int ty, int tw,
                     int th, int flags, int transparency, video_blend_mode blend_mode, int pal_offset, int pal_limit) {
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

    object_data *data = (object_data *)array->mapping;
    int row = array->item_count * 4;
    COORDS(data[row + 0], x + w, y + h, tx1, ty1, transparency, blend_mode, pal_offset, pal_limit);
    COORDS(data[row + 1], x, y + h, tx0, ty1, transparency, blend_mode, pal_offset, pal_limit);
    COORDS(data[row + 2], x, y, tx0, ty0, transparency, blend_mode, pal_offset, pal_limit);
    COORDS(data[row + 3], x + w, y, tx1, ty0, transparency, blend_mode, pal_offset, pal_limit);

    array->fans_starts[array->item_count] = array->item_count * 4;
    array->fans_sizes[array->item_count] = 4;
    array->modes[array->item_count] = blend_mode;
    array->item_count++;
}

void object_array_add(object_array *array, int x, int y, int w, int h, int tx, int ty, int tw, int th, int flags,
                      int transparency, video_blend_mode blend_mode, int pal_offset, int pal_limit) {
    if(array->item_count >= MAX_FANS) {
        PERROR("Too many objects!");
        return;
    }
    if(array->mode_flip == -1) {
        array->mode_flip = blend_mode;
    }
    float dx = 1.0f / array->src_w;
    float dy = 1.0f / array->src_h;
    add_item(array, dx, dy, x, y, w, h, tx, ty, tw, th, flags, transparency, blend_mode, pal_offset, pal_limit);
}
