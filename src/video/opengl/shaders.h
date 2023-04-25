#ifndef SHADERS_H
#define SHADERS_H

#include <epoxy/gl.h>

bool create_program(GLuint *program_id, const char *const vertex_shader, const char *const fragment_shader);
void delete_program(GLuint program_id);

#endif // SHADERS_H
