#include "video/renderers/opengl3/helpers/fbo.h"
#include "utils/log.h"
#include "video/renderers/opengl3/helpers/bindings.h"

GLuint fbo_create(GLuint texture_id) {
    GLuint id;
    glGenFramebuffers(1, &id);
    bindings_bind_fbo(id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        PERROR("Framebuffer not complete!");
    }
    bindings_unbind_fbo(id);
    return id;
}

void fbo_free(GLuint id) {
    bindings_unbind_fbo(id);
    glDeleteFramebuffers(1, &id);
}
