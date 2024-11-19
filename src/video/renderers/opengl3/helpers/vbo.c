#include "video/renderers/opengl3/helpers/vbo.h"
#include "video/renderers/opengl3/helpers/bindings.h"

GLuint vbo_create(GLsizeiptr size) {
    GLuint id;
    glGenBuffers(1, &id);
    bindings_bind_vbo(id);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    return id;
}

void *vbo_map(GLuint id, GLsizei size) {
    bindings_bind_vbo(id);
    return glMapBufferRange(GL_ARRAY_BUFFER, 0, size,
                            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
}

void vbo_unmap(GLuint id, GLsizei size) {
    bindings_bind_vbo(id);
    glFlushMappedBufferRange(GL_ARRAY_BUFFER, 0, size);
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void vbo_free(GLuint id) {
    bindings_unbind_vbo(id);
    glDeleteBuffers(1, &id);
}
