#include <assert.h>
#include <epoxy/gl.h>
#include <stdalign.h>
#include <stdlib.h>

#include "utils/allocator.h"
#include "utils/log.h"
#include "video/enums.h"
#include "video/renderers/opengl3/helpers/object_array.h"
#include "video/renderers/opengl3/helpers/vao.h"
#include "video/renderers/opengl3/helpers/vbo.h"

#define OBJ_BYTES (sizeof(object_data) * 4)
#define MAX_FANS 2048
#define VBO_SIZE (MAX_FANS * OBJ_BYTES)

typedef struct {
    GLfloat x;
    GLfloat y;
    GLfloat tex_x;
    GLfloat tex_y;
    GLint transparency;
    GLint remap_offset;
    GLint remap_rounds;
    GLint palette_offset;
    GLint palette_limit;
    GLint opacity;
    GLuint options;
} object_data;
static_assert(4 == alignof(object_data), "object_data alignment is expected to be 4");

typedef struct object_array {
    GLuint vbo_id;
    GLuint vao_id;
    GLfloat src_w; // Source texture width
    GLfloat src_h; // Source texture height
    int vbo_flip;
    int item_count;
    object_data *mapping;
    GLint fans_starts[MAX_FANS];
    GLsizei fans_sizes[MAX_FANS];
    object_array_blend_mode modes[MAX_FANS];
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
    int stride = 4 * sizeof(GLfloat) + 7 * sizeof(GLint);
    int index = 0;
    unsigned char *step = 0;
    ATTRIB(index, stride, step, 2, GL_FLOAT, GL_FALSE);
    ATTRIB(index, stride, step, 2, GL_FLOAT, GL_FALSE);
    ATTRIB_I(index, stride, step, 1, GL_INT);
    ATTRIB_I(index, stride, step, 1, GL_INT);
    ATTRIB_I(index, stride, step, 1, GL_INT);
    ATTRIB_I(index, stride, step, 1, GL_INT);
    ATTRIB_I(index, stride, step, 1, GL_INT);
    ATTRIB_I(index, stride, step, 1, GL_INT);
    ATTRIB_I(index, stride, step, 1, GL_UNSIGNED_INT);
}

object_array *object_array_create(GLfloat src_w, GLfloat src_h) {
    object_array *array = omf_calloc(1, sizeof(object_array));
    array->item_count = 0;
    array->mapping = NULL;
    array->src_w = src_w;
    array->src_h = src_h;
    array->vbo_id = vbo_create(VBO_SIZE);
    array->vao_id = vao_create();
    setup_vao_layout();
    return array;
}

void object_array_free(object_array **array) {
    object_array *obj = *array;
    if(obj != NULL) {
        vbo_free(obj->vbo_id);
        vao_free(obj->vao_id);
        omf_free(obj);
        *array = NULL;
    }
}

void object_array_prepare(object_array *array) {
    if(array->mapping != NULL) {
        PERROR("VBO is already mapped! Remember to call object_array_finish.");
        return;
    }
    array->mapping = vbo_map(array->vbo_id, VBO_SIZE);
    array->item_count = 0;
}

void object_array_finish(object_array *array) {
    vbo_unmap(array->vbo_id, array->item_count * OBJ_BYTES);
    array->mapping = NULL;
}

void object_array_begin(const object_array *array, object_array_batch *state) {
    state->start = 0;
    state->end = 0;
    state->mode = (array->item_count > 0) ? array->modes[0] : MODE_SET;
}

bool object_array_get_batch(const object_array *array, object_array_batch *state, object_array_blend_mode *mode) {
    if(state->end >= array->item_count) {
        return false;
    }
    state->start = state->end;
    object_array_blend_mode next;
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

#define COORDS(ptr, cx, cy, tx, ty, transparency, remap_offset, remap_rounds, pal_offset, pal_limit, opacity, options) \
    ptr.x = cx;                                                                                                        \
    ptr.y = cy;                                                                                                        \
    ptr.tex_x = tx;                                                                                                    \
    ptr.tex_y = ty;                                                                                                    \
    ptr.transparency = transparency;                                                                                   \
    ptr.remap_offset = remap_offset;                                                                                   \
    ptr.remap_rounds = remap_rounds;                                                                                   \
    ptr.palette_offset = pal_offset;                                                                                   \
    ptr.palette_limit = pal_limit;                                                                                     \
    ptr.opacity = opacity;                                                                                             \
    ptr.options = options;

static void add_item(object_array *array, float dx, float dy, int x, int y, int w, int h, int tx, int ty, int tw,
                     int th, int flags, int transparency, int remap_offset, int remap_rounds, int pal_offset,
                     int pal_limit, int opacity, unsigned int options) {
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
    COORDS(data[row + 0], x + w, y + h, tx1, ty1, transparency, remap_offset, remap_rounds, pal_offset, pal_limit,
           opacity, options);
    COORDS(data[row + 1], x, y + h, tx0, ty1, transparency, remap_offset, remap_rounds, pal_offset, pal_limit, opacity,
           options);
    COORDS(data[row + 2], x, y, tx0, ty0, transparency, remap_offset, remap_rounds, pal_offset, pal_limit, opacity,
           options);
    COORDS(data[row + 3], x + w, y, tx1, ty0, transparency, remap_offset, remap_rounds, pal_offset, pal_limit, opacity,
           options);

    array->fans_starts[array->item_count] = array->item_count * 4;
    array->fans_sizes[array->item_count] = 4;
    if(remap_rounds > 0) {
        array->modes[array->item_count] = MODE_REMAP;
    } else if(options & SPRITE_INDEX_ADD) {
        array->modes[array->item_count] = MODE_ADD;
    } else {
        array->modes[array->item_count] = MODE_SET;
    }
    array->item_count++;
}

void object_array_add(object_array *array, int x, int y, int w, int h, int tx, int ty, int tw, int th, int flags,
                      int transparency, int remap_offset, int remap_rounds, int pal_offset, int pal_limit, int opacity,
                      unsigned int options) {
    if(array->item_count >= MAX_FANS) {
        PERROR("Too many objects!");
        return;
    }
    float dx = 1.0f / array->src_w;
    float dy = 1.0f / array->src_h;
    add_item(array, dx, dy, x, y, w, h, tx, ty, tw, th, flags, transparency, remap_offset, remap_rounds, pal_offset,
             pal_limit, opacity, options);
}
