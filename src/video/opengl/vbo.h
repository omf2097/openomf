#ifndef VBO_H
#define VBO_H

#include <epoxy/gl.h>

GLuint vbo_create(GLsizeiptr size);
void *vbo_map(GLuint id, GLsizei size);
void vbo_unmap(GLuint id);
void vbo_free(GLuint id);

#endif // VBO_H
