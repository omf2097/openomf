#ifndef BUFFERS_H
#define BUFFERS_H

#include <epoxy/gl.h>

GLuint ubo_create(GLsizeiptr size);
void ubo_free(GLuint id);

GLuint vbo_create(GLsizeiptr size);
void vbo_free(GLuint id);

GLuint vao_create();
void vao_free(GLuint id);

GLuint texture_create(GLsizei w, GLsizei h, GLint internal_format, GLenum format);
void texture_update(int x, int y, int w, int h, const char *bytes);
void texture_free(GLuint id);

#endif // BUFFERS_H
