#ifndef PAL_BUFFER_H
#define PAL_BUFFER_H

#include <epoxy/gl.h>

typedef struct pal_buffer pal_buffer;

pal_buffer *pal_buffer_create();
GLuint pal_buffer_get_block(pal_buffer *pal_buffer);
void pal_buffer_update(pal_buffer *pal_buffer, const void *data);
void pal_buffer_free(pal_buffer **pal_buffer);

#endif // PAL_BUFFER_H
