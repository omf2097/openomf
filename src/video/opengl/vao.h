#ifndef VAO_H
#define VAO_H

#include <epoxy/gl.h>

GLuint vao_create(void);
void vao_free(GLuint id);

#endif // VAO_H
