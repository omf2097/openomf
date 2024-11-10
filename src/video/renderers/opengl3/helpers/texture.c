#include "video/renderers/opengl3/helpers/texture.h"
#include "video/renderers/opengl3/helpers/bindings.h"

GLuint texture_create(GLuint tex_unit, GLsizei w, GLsizei h, GLint internal_format, GLenum format) {
    GLuint id = 0;
    glGenTextures(1, &id);
    bindings_bind_tex(tex_unit, id);
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

void texture_update(GLuint tex_unit, GLuint id, int x, int y, int w, int h, GLenum format, const char *bytes) {
    bindings_bind_tex(tex_unit, id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, format, GL_UNSIGNED_BYTE, bytes);
}

void texture_free(GLuint tex_unit, GLuint id) {
    bindings_unbind_tex(tex_unit, id);
    glDeleteTextures(1, &id);
}
