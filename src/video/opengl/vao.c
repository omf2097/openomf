#include "video/opengl/vao.h"
#include "video/opengl/bindings.h"

GLuint vao_create(void) {
    GLuint id;
    glGenVertexArrays(1, &id);
    bindings_bind_vao(id);
    return id;
}

void vao_free(GLuint id) {
    bindings_unbind_vao(id);
    glDeleteVertexArrays(1, &id);
}
