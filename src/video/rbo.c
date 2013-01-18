#include "video/rbo.h"
#include "utils/log.h"
#include <GL/glew.h>

void rbo_create(rbo *rbo, unsigned int w, unsigned int h) {
    rbo->w = w;
    rbo->h = h;
    glGenRenderbuffers(1, &rbo->id);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo->id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
}

void rbo_free(rbo *rbo) {
    glDeleteRenderbuffers(1, &rbo->id);
}

void rbo_bind(rbo *rbo) {
    glBindRenderbuffer(GL_RENDERBUFFER, rbo->id);
}

void rbo_unbind() {
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}
