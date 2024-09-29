#ifndef BINDINGS_H
#define BINDINGS_H

#include "epoxy/gl.h"

void binding_active_tex(GLuint id);

void bindings_bind_vao(GLuint id);
void bindings_bind_vbo(GLuint id);
void bindings_bind_ubo(GLuint id);
void bindings_bind_tex(GLuint unit, GLuint id);
void bindings_bind_fbo(GLuint id);

void bindings_unbind_vao(GLuint id);
void bindings_unbind_vbo(GLuint id);
void bindings_unbind_ubo(GLuint id);
void bindings_unbind_tex(GLuint unit, GLuint id);
void bindings_unbind_fbo(GLuint id);

#endif // BINDINGS_H
