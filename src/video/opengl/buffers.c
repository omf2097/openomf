#include "video/opengl/buffers.h"
#include "video/opengl/bindings.h"

GLuint ubo_create(GLsizeiptr size) {
    GLuint id;
    glGenBuffers(1, &id);
    bindings_bind_ubo(id);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);
    return id;
}

void ubo_update(GLuint id, GLsizeiptr size, const void *data) {
    bindings_bind_ubo(id);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
}

void ubo_free(GLuint id) {
    bindings_unbind_ubo(id);
    glDeleteBuffers(1, &id);
}

GLuint texture_create(GLsizei w, GLsizei h, GLint internal_format, GLenum format) {
    GLuint id = 0;
    glGenTextures(1, &id);
    bindings_bind_tex(id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, GL_UNSIGNED_BYTE, NULL);
    return id;
}

void texture_update(GLuint id, int x, int y, int w, int h, const char *bytes) {
    bindings_bind_tex(id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RG, GL_UNSIGNED_BYTE, bytes);
}

void texture_free(GLuint id) {
    bindings_unbind_tex(id);
    glDeleteTextures(1, &id);
}

GLuint vbo_create(GLsizeiptr size) {
    GLuint id;
    glGenBuffers(1, &id);
    bindings_bind_vbo(id);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    return id;
}

void *vbo_map(GLuint id, GLsizei size) {
    bindings_bind_vbo(id);
    return glMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void vbo_unmap(GLuint id) {
    bindings_bind_vbo(id);
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void vbo_free(GLuint id) {
    bindings_unbind_vbo(id);
    glDeleteBuffers(1, &id);
}

GLuint vao_create() {
    GLuint id;
    glGenVertexArrays(1, &id);
    bindings_bind_vao(id);
    return id;
}

void vao_free(GLuint id) {
    bindings_unbind_vao(id);
    glDeleteVertexArrays(1, &id);
}