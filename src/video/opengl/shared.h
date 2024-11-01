#ifndef SHARED_H
#define SHARED_H

#include "video/enums.h"
#include "video/vga_palette.h"
#include <epoxy/gl.h>

typedef struct shared shared;

shared *shared_create(void);
void shared_set_palette(shared *shared, vga_palette *data, vga_index start, vga_index end);
GLuint shared_get_block(shared *buffer);
void shared_free(shared **buffer);

#endif // SHARED_H
