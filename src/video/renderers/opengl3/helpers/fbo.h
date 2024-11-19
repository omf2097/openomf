#ifndef FBO_H
#define FBO_H

#include <epoxy/gl.h>

GLuint fbo_create(GLuint texture_id);
void fbo_free(GLuint id);

#endif // FBO_H
