#include "video/fbo.h"
#include "utils/log.h"
#include <GL/glew.h>
#include <stdlib.h>

const char* fbo_get_status(int code) {
    switch(code) {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        return "attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        return "missing attachment";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        return "draw_buffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        return "read_buffer";
    case GL_FRAMEBUFFER_UNSUPPORTED:
        return "unsupported";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        return "layer_targets";
    case GL_FRAMEBUFFER_COMPLETE:
        return "framebuffer complete";
    default:
        return "unknown error";
    }
}

int fbo_create(fbo *fbo, unsigned int w, unsigned int h) {
    // Texture and Renderbuffer object (RBO)
    rbo_create(&fbo->rbo, w, h);
    texture_create(&fbo->tex, 0, w, h);
    
    // Create FBO
    glGenFramebuffers(1, &fbo->id);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->tex.id, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo->rbo.id);
    
    // Make sure everything worked.
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        PERROR("Unable to create fbo: %s.", fbo_get_status(status));
        return 1;
    }
    return 0;
}

void fbo_free(fbo *fbo) {
    glDeleteFramebuffers(1, &fbo->id);
    texture_free(&fbo->tex);
    rbo_free(&fbo->rbo);
}

void fbo_bind(fbo *fbo) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->id);
}

void fbo_unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
