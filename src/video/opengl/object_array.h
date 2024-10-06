#ifndef OBJECT_ARRAY_H
#define OBJECT_ARRAY_H

#include "video/enums.h"

typedef struct object_array object_array;
typedef struct {
    int start;
    int end;
    int mode;
} object_array_batch;

object_array *object_array_create(GLfloat src_w, GLfloat src_h);
void object_array_free(object_array **array);

void object_array_prepare(object_array *array);
void object_array_finish(object_array *array);
void object_array_begin(const object_array *array, object_array_batch *state);
bool object_array_get_batch(const object_array *array, object_array_batch *state);
void object_array_draw(const object_array *array, object_array_batch *state);
void object_array_add(object_array *array, int x, int y, int w, int h, int tx, int ty, int tw, int th, int flags,
                      video_blend_mode blend_mode, int pal_offset, int pal_limit);

#endif // OBJECT_ARRAY_H
