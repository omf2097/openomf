#ifndef TEXTURE_H
#define TEXTURE_H

#include <epoxy/gl.h>

GLuint texture_create(GLuint tex_unit, GLsizei w, GLsizei h, GLint internal_format, GLenum format);
void texture_update(GLuint tex_unit, GLuint id, int x, int y, int w, int h, GLenum format, const char *bytes);
void texture_free(GLuint tex_unit, GLuint id);

#endif // TEXTURE_H
