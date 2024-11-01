#include "video/opengl/ubo.h"
#include "video/opengl/bindings.h"

GLuint ubo_create(GLsizeiptr size) {
    GLuint id;
    glGenBuffers(1, &id);
    bindings_bind_ubo(id);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
    return id;
}

void ubo_update(GLuint id, GLsizeiptr offset, GLsizeiptr size, const void *data) {
    bindings_bind_ubo(id);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}

void ubo_free(GLuint id) {
    bindings_unbind_ubo(id);
    glDeleteBuffers(1, &id);
}
