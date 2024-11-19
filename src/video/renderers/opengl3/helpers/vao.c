#include "video/renderers/opengl3/helpers/vao.h"
#include "video/renderers/opengl3/helpers/bindings.h"

GLuint vao_create(void) {
    GLuint id;
    glGenVertexArrays(1, &id);
    bindings_bind_vao(id);
    return id;
}

void vao_use(GLuint id) {
    bindings_bind_vao(id);
}

void vao_free(GLuint id) {
    bindings_unbind_vao(id);
    glDeleteVertexArrays(1, &id);
}
