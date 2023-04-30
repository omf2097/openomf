#include "video/opengl/bindings.h"

static GLuint bound_ubo = 0;
static GLuint bound_vbo = 0;
static GLuint bound_vao = 0;
static GLuint bound_tex = 0;

void bindings_bind_vao(GLuint id) {
    if(bound_vao != id) {
        glBindVertexArray(id);
        bound_vao = id;
    }
}

void bindings_unbind_vao(GLuint id) {
    if(bound_vao == id) {
        glBindVertexArray(0);
        bound_vao = 0;
    }
}

void bindings_bind_vbo(GLuint id) {
    if(bound_vbo != id) {
        glBindBuffer(GL_ARRAY_BUFFER, id);
        bound_vbo = id;
    }
}

void bindings_unbind_vbo(GLuint id) {
    if(bound_vbo == id) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        bound_vbo = 0;
    }
}

void bindings_bind_ubo(GLuint id) {
    if(bound_ubo != id) {
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        bound_ubo = id;
    }
}

void bindings_unbind_ubo(GLuint id) {
    if(bound_ubo == id) {
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        bound_ubo = 0;
    }
}

void bindings_bind_tex(GLuint id) {
    if(bound_tex != id) {
        glBindTexture(GL_TEXTURE_2D, id);
        bound_tex = id;
    }
}

void bindings_unbind_tex(GLuint id) {
    if(bound_tex == id) {
        glBindTexture(GL_TEXTURE_2D, 0);
        bound_tex = 0;
    }
}
