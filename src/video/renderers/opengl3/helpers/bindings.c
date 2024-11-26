#include <assert.h>

#include "video/renderers/opengl3/helpers/bindings.h"

static GLuint bound_ubo = 0;
static GLuint bound_vbo = 0;
static GLuint bound_vao = 0;
static GLuint bound_tex[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static GLuint bound_fbo = 0;
static GLuint active_tex = ~0u;

void binding_active_tex(GLuint unit_id) {
    if(active_tex != unit_id) {
        glActiveTexture(GL_TEXTURE0 + unit_id);
        active_tex = unit_id;
    }
}

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

void bindings_bind_tex(GLuint unit, GLuint id) {
    assert(unit < 16);
    binding_active_tex(unit);
    if(bound_tex[unit] != id) {
        glBindTexture(GL_TEXTURE_2D, id);
        bound_tex[unit] = id;
    }
}

void bindings_unbind_tex(GLuint unit, GLuint id) {
    assert(unit < 16);
    binding_active_tex(unit);
    if(bound_tex[unit] == id) {
        glBindTexture(GL_TEXTURE_2D, 0);
        bound_tex[unit] = 0;
    }
}

void bindings_bind_fbo(GLuint id) {
    if(bound_fbo != id) {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        bound_fbo = id;
    }
}

void bindings_unbind_fbo(GLuint id) {
    if(bound_fbo == id) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        bound_fbo = 0;
    }
}
