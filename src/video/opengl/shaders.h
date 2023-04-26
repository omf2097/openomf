#ifndef SHADERS_H
#define SHADERS_H

#include <epoxy/gl.h>

bool create_program(GLuint *program_id, const char *const vertex_shader, const char *const fragment_shader);
void delete_program(GLuint program_id);
void activate_program(GLuint program_id);
void bind_uniform_4fv(GLuint program_id, const char *name, GLfloat *data);

#endif // SHADERS_H
