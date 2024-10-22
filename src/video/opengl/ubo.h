#ifndef UBO_H
#define UBO_H

#include <epoxy/gl.h>

GLuint ubo_create(GLsizeiptr size);
void ubo_update(GLuint id, GLsizeiptr size, const void *data);
void ubo_free(GLuint id);

#endif // UBO_H
