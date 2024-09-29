#ifndef SHARED_H
#define SHARED_H

#include "video/enums.h"
#include <epoxy/gl.h>

typedef struct shared shared;

shared *shared_create();
void shared_set_palette(shared *shared, const void *data);
void shared_flush_dirty(shared *shared);
GLuint shared_get_block(shared *buffer);
void shared_free(shared **buffer);

#endif // SHARED_H
