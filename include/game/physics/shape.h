#ifndef _SHAPE_H
#define _SHAPE_H

#include "utils/vec.h"

enum {
    SHAPE_TYPE_POINT = 1,
    SHAPE_TYPE_RECT,
    SHAPE_TYPE_POINTGROUP,
    SHAPE_TYPE_CIRCLE,
};

typedef struct shape_t {
    void *data;
    int type;
    void (*free)(void *d);
} shape;

void shape_free(shape *shape);

#endif // _SHAPE_H